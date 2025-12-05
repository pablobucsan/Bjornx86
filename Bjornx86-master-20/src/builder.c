


#include "../include/builder.h"
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <inttypes.h>


#define RAX (gpr->registers[0])
#define EAX (gpr->registers[0]->subRegisters[0])

#define RBX (gpr->registers[1])
#define EBX (gpr->registers[1]->subRegisters[0])

#define RCX (gpr->registers[2])
#define ECX (gpr->registers[2]->subRegisters[0])

#define RDX (gpr->registers[3])
#define EDX (gpr->registers[3]->subRegisters[0])



RegisterTable *gpr = NULL;
RegisterTable *fpr = NULL;
Matcher *matcher = NULL;
WorkingContext workingContext;


// Would be -8 because of the address pushed by the kernel, but when doing call _main, it would be -16 which is effectively the same as 0
RSPTracker rspTracker = {.modifications_stack = {0}, .index = -1, .is_active = 0};

RequestOffice rqOffice = {.spill_count = 0};




CurrentFrameAndFunction CFF = { .stack_base_ptr = 0, .f = NULL };


/** -------------------------------- UTILS ----------------------------------------- */




int min(int a, int b)
{
    if (a < b) { return a; }
    return b;
}

int max(int a, int b)
{
    if (a > b) { return a; }
    return b;
}


/**
 * Write it in decimal, much easier
 */
void write_string(FILE *asm_file, char *str)
{
    for (int i = 0; str[i] != '\0'; i++)
    {
        fprintf(asm_file, "%i,", str[i]);
    }

    fprintf(asm_file, "0\n");
}



int node_requires_aligned_rsp(ASTNode *node)
{
    switch(node->node_type)
    {
        case NODE_STDALONE_FUNC_CALL:
        case NODE_FUNC_CALL: return 1;

        default: return 0;
    }
}

void start_tracking_rsp()
{


    if (rspTracker.index + 1 >= MAX_NESTING)
    {
        fprintf(stderr,"Max nesting surpased: %i.Can't start tracking rsp \n", MAX_NESTING);
        exit(1);
    }


    rspTracker.index++;
    rspTracker.is_active = 1;

    rspTracker.modifications_stack[rspTracker.index].n = 0;
    rspTracker.modifications_stack[rspTracker.index].has_been_modified = 0;

    //printf("START TRACKING RSP. INDEX IS NOW: %i\n", rspTracker.index);

}

void stop_tracking_rsp()
{
    rspTracker.index--;

    if (rspTracker.index == -1)
    {
        rspTracker.is_active = 0;
    }

    if (rspTracker.index < -1)
    {
        fprintf(stderr, "After stopping tracking rsp, index = %i.Not allowed index.\n",rspTracker.index);
        exit(1);
    }

    //printf("STOP TRACKING RSP. INDEX IS NOW: %i\n", rspTracker.index);

}

void modify_rsp(int n)
{
    rspTracker.modifications_stack[rspTracker.index].n += n;
}


/*———————————————————————————————————---- REGISTER REQUESTING, SPILLING AND LOGGING —————————————————————————————————————————————————————————  */



void log_spill(Register *reg)
{
    // Create a SpillRecord 
    SpillRecord spill_record = {.is_restored = 0, .reg = reg};

    // Log/Add it to the stack
    rqOffice.spills[rqOffice.spill_count] = spill_record;

    // Increment the spill count
    rqOffice.spill_count++;
}

/**
 * 
 * Spill/Push 'reg' to the stack
 * Also unpins it since now it can be used again
 */
void spill_reg(FILE *asm_file, Register *reg)
{
    // Get the parent that will be spilled
    Register *parent = getParent64(reg);

    // Don't spill twice the same register
    if (parent->is_spilled)
    {
        fprintf(stderr, "In 'spill_reg()': Can't spill twice the same register: %s\n", reg->name);
        exit(1);
    }

    // Mark it as spilled and the children too
    parent->is_spilled = 1;
    for (int i = 0; i < 3; i++)
    {
        if (parent->subRegisters[i] != NULL)
        {
            parent->subRegisters[i]->is_spilled = 1;
        }
    }
    // Push it to the stack
    emit_push(asm_file, parent->name);

    // No longer pinned
    unpinRegister(parent);

    // Log the spill
    log_spill(reg);
}

/**
 * 
 * Collects/Pops the spilled registers that are NOT pinned
 */
void collect_spilled_regs(FILE *asm_file)
{
    if (rqOffice.spill_count > 1)
    {
        emit_comment(asm_file, "Collecting spilled registers");
    }
    for (int i = rqOffice.spill_count - 1; i >= 0; i--)
    {
        // If is spilled AND is NOT pinned, then we collect it
        if (rqOffice.spills[i].reg->is_spilled && !rqOffice.spills[i].reg->is_pinned)
        {
            emit_pop(asm_file, rqOffice.spills[i].reg->name);
            rqOffice.spills[i].reg->is_spilled = 0;
        }
    }
}



/**
 *  Pins a register and also the entire family
 */
void pinRegister(Register *reg)
{
    Register *parent = getParent64(reg);

    parent->is_pinned = 1;
    for (int i = 0; i < 3; i++)
    {
        if (parent->subRegisters[i] != NULL)
        {
            parent->subRegisters[i]->is_pinned = 1;
        }
    }
}

/**
 * Unpins a register and also the entire family
 */
void unpinRegister(Register *reg)
{

    Register *parent = reg->parent64Register != NULL? reg->parent64Register : reg;

    parent->is_pinned = 0;
    for (int i = 0; i < 3; i++)
    {
        if (parent->subRegisters[i] != NULL)
        {
            parent->subRegisters[i]->is_pinned = 0;
        }
    }
}

/**
 * 
 * Returns the specific given register.
 * It does not pin the returned register
 * It DOES spill to the stack. 
 */
Register *request_specific_reg(FILE *asm_file, Register *reg)
{
    
    // If it's not in use, get it
    if (!reg->is_pinned) { return reg; }

    // Ensure we have the parent 
    Register *parent = getParent64(reg);

    // Push to the stack 
    emit_push(asm_file, parent->name);

    // Log the pushed registers
    log_spill(parent);

    // Return the register requested
    return reg;
}


/**
 * 
 * Returns a register (any) matching the EXPECTED RegisterType and GIVEN size.
 * It does not pin the returned register.
 * It does not spill to the stack. Stops execution on register allocation failure. 
 */
Register *request_unspilled_specific_size_reg(int size)
{
    // Current expectation. Are we expecting a specific register type and size?
    Expectation *exp = getCurrentExpectation();

    // If we aren't expecting a specific register type and size, does it make sense to request a register?
        // It does, for number literals ---> Handle this case.
        if (exp == NULL)
        {
            return getUnspilledGPR(size);
        }

        int index = matcher->current_expectation_index - 1; // Current expectation index
        int size_of_exp = exp->size;


        // If the current register expectation is ANY, traverse the expectations stack to find 
        // an expecation that has been pushed before whose register type is not ANY, if there is
        while (exp->registerType == ANY && index > 0)
        {
            index--;
            exp = matcher->expecations_to_match[index];
        }


        // At this point we are guaranteed to have either ANY => no other expectations were pushed
        // or another register type. If it's any, return GPR

        if (exp->registerType == ANY || exp->registerType == GPR)
        {
            return getUnspilledGPR(size_of_exp);
        }


        // Otherwise makes no sense, FPRs are not spilled
        fprintf(stderr, "In 'request_unspilled_specific_size_reg()': Cannot get an FPR unspilled register for size: %i\n", size);
        exit(1);
}

/**
 * 
 * Returns a register (any) matching the EXPECTED RegisterType and GIVEN size.
 * It does not pin the returned register.
 * It does not spill to the stack. Stops execution on register allocation failure. 
 */
Register *request_specific_size_reg(int size)
{

    // Current expectation. Are we expecting a specific register type and size?
    Expectation *exp = getCurrentExpectation();


    // If we aren't expecting a specific register type and size, does it make sense to request a register?
    // It does, for number literals ---> Handle this case.
    if (exp == NULL)
    {
        return getGPR(size);
    }

    int index = matcher->current_expectation_index - 1; // Current expectation index
    int size_of_exp = exp->size;


    // If the current register expectation is ANY, traverse the expectations stack to find 
    // an expecation that has been pushed before whose register type is not ANY, if there is
    while (exp->registerType == ANY && index > 0)
    {
        index--;
        exp = matcher->expecations_to_match[index];
    }


    // At this point we are guaranteed to have either ANY => no other expectations were pushed
    // or another register type. If it's any, return GPR

    if (exp->registerType == ANY || exp->registerType == GPR)
    {
        return getGPR(size_of_exp);
    }


    // Otherwise, get an FPR 
    return getFPR(size_of_exp);

}


/**
 * 
 * Returns a register (any) matching the GIVEN RegisterType and size.
 * It does not pin the returned register.
 * It does not spill to the stack. Stops execution on register allocation failure. 
 */
Register *request_specific_size_and_type_reg(RegisterType rType, int size)
{
    // The returning register
    Register *result;

    switch (rType)
    {
        case ANY:
        case GPR: return getGPR(size);
        case FPR: return getFPR(size);
    }

}



/**
 * Returns the parent 64 bits register if there is one, otherwise returns itself
 */
Register *getParent64(Register *reg)
{
    if (reg->parent64Register == NULL) { return reg; }

    return reg->parent64Register;
}


/**
 * Returns 1 if both registers are family, that is, have the same parent
 * Otherwise returns 0
 */
int are_reg_family(Register *a, Register *b)
{
    Register *parentA = (a->parent64Register == NULL) ? a : a->parent64Register;
    Register *parentB = (b->parent64Register == NULL) ? b : b->parent64Register;

    return parentA == parentB;
}

/**
 * 
 * Returns a register from the same family for the specified size
 */
Register *getFamilyRegWithSize(Register *reg, int size)
{
    Register *parent = getParent64(reg);

    if (parent->size == size) { return parent; }

    for (int i = 0; i < 3; i++)
    {
        if (parent->subRegisters[i]->size == size) { return parent->subRegisters[i]; }
    }

    fprintf(stderr, "There is no family register for reg register: %s, with size: %i \n", reg->name, size);
    exit(1);
}

/**
 * 
 * From the two passed registers, the smaller size is promoted, that is,
 * the register with the bigger size is swapped for one of the same family with the smaller size
 */
void promote_smaller_reg_size(Register **regA, Register **regB)
{
    if ((*regA)->size != (*regB)->size)
    {
        int min_size = min((*regA)->size, (*regB)->size);

        *regA = getFamilyRegWithSize(*regA, min_size);
        *regB = getFamilyRegWithSize(*regB, min_size);
    }
}


/**
 * 
 * From the two passed registers, the bigger size is promoted, that is,
 * the register with the bigger size is swapped for one of the same family with the bigger size
 */
void promote_bigger_reg_size(Register **regA, Register **regB)
{
    if ((*regA)->size != (*regB)->size)
    {
        int min_size = max((*regA)->size, (*regB)->size);

        *regA = getFamilyRegWithSize(*regA, min_size);
        *regB = getFamilyRegWithSize(*regB, min_size);
    }
}



/*———————————————————————————————————---- END REGISTER REQUESTING, SPILLING AND LOGGING —————————————————————————————————————————————————————————  */





/** -------------------------------- END UTILS ----------------------------------------- */









/** -------------------------------- EMIT ----------------------------------------- */

// Writes the comment to the asm file with \n at the end
void emit_comment(FILE *asm_file, char *comment)
{
    fprintf(asm_file, "    ;%s\n", comment);
}



void emit_mov_imm2r(FILE *asm_file, Register *dst, int64_t n)
{
    fprintf(asm_file, "    mov %s, %"PRId64"\n", dst->name, n);
}

/**
 * Emits an operation mov (movzx/movsx) regA, regB ensuring regA is of size 'size_to_preserve'
 * Moves the value in src to dst
 */
void emit_mov_r2r(FILE *asm_file, Operand *op_dst, Operand *op_src, int bytes_to_move)
{   
    //printf("GOT HERE \n");
    // Get the destination register able to hold the size to preserve
    Register *sized_dst = getFamilyRegWithSize(op_dst->linkedToRegister, bytes_to_move);

    //printf("SIZED DST = %s \n", sized_dst->name);
    // If they have the same size, just move it
    // Or if dst = r64 and src = r32 the instruction is the same
    if (sized_dst->size == op_src->linkedToRegister->size || (sized_dst->size == 8 && op_src->linkedToRegister->size == 4))
    {
        fprintf(asm_file, "    mov %s, %s\n", op_dst->linkedToRegister->name, op_src->linkedToRegister->name);
        return;
    }



    // At this point, we have two registers with different sizes, we have to extend src onto dst
    // Instruction is different whether we want to preserve signed or unsigned bits

    char *movOperation = (op_src->is_unsigned)? "movzx" : "movsx";

    fprintf(asm_file, "    %s %s, %s\n", movOperation ,sized_dst->name, op_src->linkedToRegister->name);




}


/**
 * Emits an operation mov regA, [regB]
 * Moves the value in the address stored in src to dst
 */
void emit_mov_a2r(FILE *asm_file, Operand *op_dst, Operand *op_src, int bytes_to_read)
{
    Register *sized_reg = getFamilyRegWithSize(op_dst->linkedToRegister, bytes_to_read);

    fprintf(asm_file, "    mov %s, [%s]\n", sized_reg->name, op_src->linkedToRegister->name);
}

/**
 * Emits an operation mov regA, [rbp - offset]
 * Moves the value in the stack to regA
 */
void emit_mov_s2r(FILE *asm_file, Operand *op_dst, int stack_offset, int bytes_to_read, int is_unsigned)
{
    // If the destination size matches the amount of bytes we want to read from the value stored in the stack, we good
    if (op_dst->linkedToRegister->size == bytes_to_read)
    {
        fprintf(asm_file, "    mov %s, [rbp - %i]\n", op_dst->linkedToRegister->name, stack_offset);
        return;
    }

    // At this point the destination size doesnt match the amount of bytes to read, we must extend
    char *movOperation = (is_unsigned)? "movzx" : "movsx";
    char *word = getWordForSize(bytes_to_read);

    fprintf(asm_file, "    %s %s, %s [rbp - %i]\n", movOperation, op_dst->linkedToRegister->name, word, stack_offset);
}

/**
 * Emits an operation mov [rbp - offset], reg
 * Moves the value in the register to the stack
 */
void emit_mov_r2s(FILE *asm_file, Operand *op_src, int stack_offset, int bytes_to_move)
{
    // If the source register size matches the amount of bytes we want to move, we are fine
    if (op_src->linkedToRegister->size == bytes_to_move)
    {
        fprintf(asm_file, "    mov [rbp - %i], %s\n", stack_offset, op_src->linkedToRegister->name);
        return;
    }

    // At this point, the source register size does not match the amount of bytes we want to move
    // Get the family register that does

    Register *sized_reg = getFamilyRegWithSize(op_src->linkedToRegister, bytes_to_move);

    fprintf(asm_file, "    mov [rbp - %i], %s\n", stack_offset, sized_reg->name);
}


// 24/11 - Change this? 
void emit_mul(FILE *asm_file, Operand left, Operand right )
{
    // If expected size is 8 bits, for example: int8 x = 3 * 4, we could end up having
    // imul al,bl which is not valid x86 syntax

    int rax_busy = 0;

    if (left.linkedToRegister->size == 1 && right.linkedToRegister->size == 1)
    {
        // Check if RAX is busy, if so, push it
        if (gpr->registers[0]->is_pinned)
        {
            fprintf(asm_file, "    push rax\n");
            rax_busy = 1;
        }

        // Check if left operand is in AL already, if so we dont need to move it
        int checkIfLeftInAL = strcmp(left.linkedToRegister->name, "al") == 0 ? 1 : 0;

        if (!checkIfLeftInAL) { fprintf(asm_file, "    mov al, %s \n", left.linkedToRegister->name); }
        
        fprintf(asm_file, "    imul %s \n", right.linkedToRegister->name);
        if (!checkIfLeftInAL) { fprintf(asm_file, "    mov %s, al \n", left.linkedToRegister->name); }

        if (rax_busy) 
        {
            fprintf(asm_file, "    pop rax\n");
        }
    }

    else 
    {
        fprintf(asm_file, "    imul %s, %s\n", left.linkedToRegister->name, right.linkedToRegister->name);
    }
}

// Performs multiply with imul r,r,imm
// Wonderful
void emit_mul_imm(FILE *asm_file, Operand left, int number)
{
    fprintf(asm_file, "    imul %s, %s, %i \n", left.linkedToRegister->name, left.linkedToRegister->name, number);
}


/* WATCH OUT FOR THIS OPERATION TOO BC HOLY SHIT
    PROMOTE TO 32 BITS AND FOLLOW THE SAME PATTERN

    DIVIDEND -> EDX:EAX
    QUOTIENT -> EAX 
    REMAINDER -> EDX

    EDX:EAX / EBX = EAX (QUOTIENT), EDX (REMAINDER)
*/
void emit_div(FILE *asm_file, Operand *left, Operand *right)
{
    // Request EAX usage
    Register *eax = request_specific_reg(asm_file, EAX);

    // Let NODE_BIN_OP :: CASE DIV/MOD: pin EAX or EDX as needed
    //pinRegister(eax);

    // Check whether they are signed or not
    int atLeastOneSigned = !left->is_unsigned || !right->is_unsigned;

    // Move dividend to EAX , ensure there is no garbage sitting there
    if (left->linkedToRegister != EAX)
    {
        fprintf(asm_file, "    movzx eax, %s\n", left->linkedToRegister->name);

        // Unpin left, its value is in EAX now
        unpinRegister(left->linkedToRegister);

        // If left was in EBX, free it so we dont push it next
        if (are_reg_family(left->linkedToRegister, EBX)) 
        { 
            unpinRegister(EBX); 
        }
    }

    char *divCode = "div";

    // If at least one is signed, perform cqo and idiv and dont clear edx
    if (!left->is_unsigned || !right->is_unsigned)
    {
        divCode = "idiv";

        // Sign extend RAX into RDX 
        fprintf(asm_file, "    cqo\n");
    }

    if (!atLeastOneSigned)
    { 
        // Clear EDX 
        fprintf(asm_file, "    xor edx, edx\n");
    }


    // Request EBX usage 
    Register *ebx = request_specific_reg(asm_file, EBX);
    // Pin EBX 
    pinRegister(ebx);
    // Ensure there is no garbage in EBX
    if (right->linkedToRegister != EBX)
    {
        fprintf(asm_file, "    movzx ebx, %s \n", right->linkedToRegister->name);
    }

    // Unpin right, its value is now in EBX
    unpinRegister(right->linkedToRegister);

    fprintf(asm_file, "    %s ebx\n", divCode);

    // Unpin EBX, the results are now in EAX (quotient) and EDX (remainder)
    unpinRegister(ebx);
}




void emit_extern_function(FILE *asm_file, char *func_signature)
{
    fprintf(asm_file, "extern %s\n", func_signature);
}

void emit_label(FILE *asm_file, char *label)
{
    fprintf(asm_file, "%s:\n", label);
}


// Performs push r8 --> push rax for example
void emit_push(FILE *asm_file, char *reg_name)
{
    // Perform the push
    fprintf(asm_file, "    push %s\n", reg_name);

    // Keep track of the modified RSP
    modify_rsp(-8);
}

// Performs pop r8 --> pop rax for example
void emit_pop(FILE *asm_file, char *reg_name)
{

    // Perform the pop
    fprintf(asm_file, "    pop %s\n", reg_name);

    // Keep track of the modified RSP
    modify_rsp(8);
}

// Performs add rsp, n
void emit_add_rsp(FILE *asm_file, uint32_t n)
{
    // Perform the addition
    fprintf(asm_file, "    add rsp, %i\n",n);

    // Keep track of the modified RSP
    modify_rsp(n);
}

// Performs sub rsp, n
void emit_sub_rsp(FILE *asm_file, uint32_t n)
{
    // Perform the substraction
    fprintf(asm_file, "    sub rsp, %i\n",n);

    // Keep track of the modified RSP
    modify_rsp(-n);
}

// Performs call label
void emit_call(FILE *asm_file, char *label)
{
    // Perform the call 
    fprintf(asm_file, "    call %s\n", label);

    // call and ret complement each other
    // Keep track of the modified RSP
    //modify_rsp(-8);
}


// Perform return
void emit_ret(FILE *asm_file)
{
    // Perform the return
    fprintf(asm_file, "    ret\n");

    // call and ret complement each other
    // Keep track of the modified RSP
    //modify_rsp(8);
}


void emit_prologue(FILE *asm_file)
{

    // Push rbp
    emit_push(asm_file, "rbp");


    // Mov into rbp, rsp
    fprintf(asm_file, "    mov rbp, rsp\n");


    // Space for the locals
    emit_sub_rsp(asm_file, getStackBasePtr());


}

// Emit function epilogue
void emit_epilogue(FILE *asm_file)
{

    emit_comment(asm_file, "Restore the stack pointer and return");

    // Unreserve the space for the locals
    emit_add_rsp(asm_file, getStackBasePtr());

    // Pop rbp 
    emit_pop(asm_file, "rbp");

    // Return 
    emit_ret(asm_file);


}


// Fine tune this function, doesn't have to add or sub the whole number rsp has been modified by. Just +- 8 (or nothing)
// if the modified value is multiple of 16.
// Gotta look into this
void emit_ensure16byte_rsp(FILE *asm_file)
{
    if (!rspTracker.is_active) { return; }

    int n = rspTracker.modifications_stack[rspTracker.index].n;

    // Its been modified by a multiple of 16 so we are golden
    if (n % 16 == 0) { return; }


    emit_comment(asm_file, "Ensuring rsp is 16 byte aligned");


    rspTracker.modifications_stack[rspTracker.index].has_been_modified = 1;
    emit_sub_rsp(asm_file, 8);
}

// 
void emit_restore_rsp(FILE *asm_file, int args_pushed)
{
    if (!rspTracker.is_active) { return; }


    // If it hasnt been modified and we havent pushed args, there is nothing to restore
    if (rspTracker.modifications_stack[rspTracker.index].has_been_modified == 0 && args_pushed == 0) { return; }


    // Otherwise
    emit_comment(asm_file, "Restoring rsp");

    int has_been_modified = rspTracker.modifications_stack[rspTracker.index].has_been_modified;

    rspTracker.modifications_stack[rspTracker.index].has_been_modified = 0;

    emit_add_rsp(asm_file, 8 * (args_pushed + has_been_modified));
}



// ----------------------------------- REASSIGNMENTS ----------------------------------------------------

// Only call this function from within emit_reassign()
void emit_memcpy(FILE *asm_file, Operand *lvalue, Operand *rvalue, ReassignmentNode *reassignment_node, SymbolTable *current_st, FunctionTable *current_ft)
{
    /**
     * Compute the LValue address we want to write to
     */
    emit_comment(asm_file, "LValue");
    *lvalue = with_lvalue_context(asm_file, reassignment_node->lvalue, current_st, current_ft);

    /**
     * Compute the RValue address we want to read from
     */
    emit_comment(asm_file, "RValue (address)");
    *rvalue = with_lvalue_context(asm_file, reassignment_node->expression, current_st, current_ft);

    /**
     * Copy the memory
     */
    emit_comment(asm_file, "Copying memory");

    fprintf(asm_file, "    lea rdi, [%s]\n", lvalue->linkedToRegister->name);
    fprintf(asm_file, "    lea rsi, [%s]\n", rvalue->linkedToRegister->name);
    fprintf(asm_file, "    mov rcx, %i\n", reassignment_node->size);
    fprintf(asm_file, "    rep movsb\n");

    // Unpin both lvalue and rvalue registers, no longer needed, the copy was already made
    unpinRegister(lvalue->linkedToRegister);
    unpinRegister(rvalue->linkedToRegister);

}

// Only call this function from within emit_reassign()
void emit_ptr_reassign(FILE *asm_file, Register *left, Register *right, int pointed_size, TokenType REASSIGN_TOKEN)
{
    switch(REASSIGN_TOKEN)
    {
        case TOKEN_ASSIGN:
        {
            fprintf(asm_file, "    mov [%s], %s \n", left->name, right->name);
            break;
        }

        case TOKEN_ADD_ASSIGN:
        {
            fprintf(asm_file, "    imul %s, %i \n", right->name, pointed_size);
            fprintf(asm_file, "    add [%s], %s \n", left->name, right->name);
            break;
        }

        case TOKEN_SUB_ASSIGN:
        {
            fprintf(asm_file, "    imul %s, %i \n", right->name, pointed_size);
            fprintf(asm_file, "    sub [%s], %s \n", left->name, right->name);
            break;
        }

        default:
        {
            fprintf(stderr, "Ptr reassignment kind not supported \n");
            exit(1);
        }
    }
}

// Only call this function from within emit_reassign()
void emit_scalar_reassign(FILE *asm_file, Register *left, Register *right, int reassignment_size, TokenType REASSIGN_TOKEN)
{
     switch(REASSIGN_TOKEN)
    {
        case TOKEN_ASSIGN:
        {
            fprintf(asm_file, "    mov %s [%s], %s \n", getWordForSize(right->size), left->name, right->name);
            break;
        }

        case TOKEN_ADD_ASSIGN:
        {
            fprintf(asm_file, "    add [%s], %s \n", left->name, right->name);
            break;
        }

        case TOKEN_SUB_ASSIGN:
        {
            fprintf(asm_file, "    sub [%s], %s \n", left->name, right->name);
            break;
        }

        default:
        {
            fprintf(stderr, "Scalar reassignment kind not supported \n");
            exit(1);
        }
    }
}


void emit_reassign(FILE *asm_file, Operand *lvalue, Operand *rvalue, ReassignmentNode *reassignment_node, SymbolTable *current_st, FunctionTable *current_ft)
{

    
    /**
     * 
     * Later change to program->reassignment_node.size > 16 
     * use  registers XMM to pass the value
     */


    // If big struct, mempcy
    if (reassignment_node->size > 8) 
    { 
        return emit_memcpy(asm_file, lvalue, rvalue, reassignment_node, current_st, current_ft); 
    }

    /*
    * Compute lvalue and rvalue
    */
    *lvalue = with_lvalue_context(asm_file, reassignment_node->lvalue, current_st, current_ft);
    *rvalue = with_rvalue_context(asm_file, reassignment_node->expression, GPR, reassignment_node->size, current_st, current_ft);


    /**
     *  Handle the case where lvalue and rvalue are put into the same register.
     *  For example: 
     *  Reassignment -->  x = foo();  
     *  That translates to:
     *  lea rax, [rbp - x.offset]
     *  call _foo               ;  Return in rax, so rax needs to be pushed before placing the return in rax
     *                          ; Problem: rax holds the return value from foo() so we can't pop into rax
     * 
     *  mov byte [rax], rax     ; Now this basically means mov byte [foo()], foo(). Wrong
     */ 

    if (are_reg_family(lvalue->linkedToRegister, rvalue->linkedToRegister))
    {
        // Request a temporary register to move there rvalue
        Register *temp = request_specific_size_reg(rvalue->linkedToRegister->size);
        fprintf(asm_file, "    mov %s, %s \n", temp->name, rvalue->linkedToRegister->name);

        // Unpin rvalue, no longer needed, its value is now in temp
        unpinRegister(rvalue->linkedToRegister);

        rvalue->linkedToRegister = temp;
    }

    // Collect the spilled registers
    collect_spilled_regs(asm_file);


    // Pointer reassignment
    if (is_type_ptr(reassignment_node->type)) 
    { 
        emit_ptr_reassign(asm_file, lvalue->linkedToRegister, rvalue->linkedToRegister, size_of_type(resolve_final_ptr_type(reassignment_node->type,1)), reassignment_node->op);
        return;  
    } 

    // Scalar reassignment
    emit_scalar_reassign(asm_file, lvalue->linkedToRegister, rvalue->linkedToRegister, reassignment_node->size, reassignment_node->op);

    // Unpin both lvalue and rvalue, no longer needed, we are done reassigning

    unpinRegister(lvalue->linkedToRegister);
    unpinRegister(rvalue->linkedToRegister);

}


// ----------------------------------- REASSIGNMENTS ----------------------------------------------------


void emit_postfix(FILE *asm_file, int is_ptr, int size, Operand operand, PostfixOPCode postfixOPCode)
{
    /**
    * For non pointer types, we increment and decrement one
    */
    if (!is_ptr)
    {

        switch (postfixOPCode)
        {
            case POSTFIX_INC:
            {

                fprintf(asm_file, "    inc %s [%s] \n", getWordForSize(size), operand.linkedToRegister->name);
                break;
            }

            case POSTFIX_DEC:
            {
                fprintf(asm_file, "    dec %s [%s] \n", getWordForSize(size), operand.linkedToRegister->name);
                break;
            }
        }
    }

    /**
    * For pointer types, we increment and decrement by the size of the type pointed to
    */
    else 
    {
        char *word = (size > 8) ? "qword" : getWordForSize(size);
        switch (postfixOPCode)
        {
            case POSTFIX_INC:
            {
                fprintf(asm_file, "    add %s [%s], %i \n", word, operand.linkedToRegister->name, size);
                break;
            }

            case POSTFIX_DEC:
            {
                fprintf(asm_file, "    sub %s [%s], %i \n", word,operand.linkedToRegister->name, size);
                break;
            }
        }
    }
}



void emit_binop_deepest(FILE *asm_file, BinaryOPNode *binary_op_node, Operand *left, Operand *right, SymbolTable *current_st, FunctionTable *current_ft)
{
    
    /**
     * Compute the binary operation depth. The deepest branch will be evaluated first so we dont run out of registers
     */
    int left_depth = compute_binaryOperationDepth(binary_op_node->left, 1);
    int right_depth = compute_binaryOperationDepth(binary_op_node->right, 1);
    

    if (right_depth > left_depth)
    {
        *right = buildStart(asm_file, binary_op_node->right, current_st, current_ft);
        *left = buildStart(asm_file, binary_op_node->left, current_st, current_ft);
    }

    else 
    {
        *left = buildStart(asm_file,binary_op_node->left, current_st, current_ft);
        *right = buildStart(asm_file, binary_op_node->right, current_st, current_ft);
    }
    
    unpinRegister(right->linkedToRegister);
    pinRegister(left->linkedToRegister);
}


//**  -------------------------------------- END EMIT --------------------------------------------- **//

void updateCFF(Function *f)
{
    CFF.f = f;
    // RSP has to be subtracted enough to account for the weight of all variables AND the padding in between them
    //printf("FUNCTION: %s, has weight: %i, and padding: %i \n", f->identifier, f->local_symbols->weight, f->local_symbols->padding);
    CFF.stack_base_ptr = align_rsp(f->local_symbols->weight + f->local_symbols->padding);

    //printf("UPDATED CFF. Stack Base Pointer = %i\n", CFF.stack_base_ptr);

    //printf("Total weight and padding for function %s : %i + %i = %i\n", f->identifier,f->local_symbols->weight, f->local_symbols->padding,
    //        f->local_symbols->weight + f->local_symbols->padding);
}

Function *getCurrentFunction() { return CFF.f; }

int getStackBasePtr() { return CFF.stack_base_ptr; }


void enter_node(ASTNode *node)
{
    //printf("ENTERING NODE %s\n", astTypeToStr(node));
    // 1. If the node is intended to push a scope, push it 
    if (is_pushing_scope(node))
    {
        push_bt_context(astToContext(node));
    }
    

    // 2. Clear the registers, we may be pushing a scope within another one
    if (is_statement(node))
    {
        clearRegisters(gpr);
        clearRegisters(fpr);
        clearAllExpectations();
        popAllWorkingContexts();
    }


}

// 1.Pop bt_context if scope has been pushed 
// 2.Clear all registers if the node is a complete statement 
// 3.Clear all expectations, there is nothing to expect anymore 
// 4.Pop all working contexts, we aren't working in anything 
void exit_node(ASTNode *node)
{
    //printf("EXITING NODE %s\n", astTypeToStr(node));

    // If the node pushes a scope, pop it as we exit it 
    if (is_pushing_scope(node))
    {
        pop_bt_context();
    }

    // If the node is a statement, as soon as we leave it, whatever value is left in a register is no longer needed, clear the registers
    // If the node is a statement, as soon as we leave it, there are no expectations left to match, clear them all
    // If the node is a statement, as soon as we leave it, there are no meaningful working contexts, pop them all
    if (is_statement(node))
    {
        clearRegisters(gpr);
        clearRegisters(fpr);
        clearAllExpectations();
        popAllWorkingContexts();
    }
    
    // If the node is a func call, clear the fpr as soon as we leave it, no longer need it. Further func calls within the 
    // statement can reuse the registers 
    if (node->node_type == NODE_FUNC_CALL)
    {
        clearRegisters(fpr);
    }
}



Operand with_lvalue_context(FILE *asm_file, ASTNode *node, SymbolTable *st, FunctionTable *ft)
{
    pushWorkingContext(COMPUTING_LVALUE);
    expectRegister(ANY, 8);
    Operand result = buildStart(asm_file, node, st, ft);
    clearExpectation();
    popWorkingContext(); 
    return result;
}

Operand with_rvalue_context(FILE *asm_file, ASTNode *node, RegisterType rtype, int size, SymbolTable *st, FunctionTable *ft)
{

    pushWorkingContext(COMPUTING_RVALUE);
    expectRegister(rtype, size);
    Operand result = buildStart(asm_file, node, st, ft);
    clearExpectation();
    popWorkingContext();
    return result;
}


ContextType astToContext(ASTNode *node)
{
    switch(node->node_type)
    {
        case NODE_FUNC_DEF: return CTX_FUNCTION;
        case NODE_IF: return CTX_IF;
        case NODE_WHILE: return CTX_WHILE;
        case NODE_FOR: return CTX_FOR;
        case NODE_FOREACH: return CTX_FOREACH;
        default: 
        {
            fprintf(stderr, "Don't know what ContextType corresponds to ASTNode type: %s \n", astTypeToStr(node));
            exit(1);
        }
    }
}


// Reserve stack for variables. There's already 8 when doing push rbp. Align to 16-byte  
//
// align_size + 8 = 16 * m, where m natural >= 1. align_size >= weight 
// best case scenario: weight + 8 is a multiple of 16 => no over allocation 
// worst case sceneario: weight + 8 = 16*m - 1 for a given m, then there is 15 bytes of over-allocation 
int align_rsp(int locals_weight)
{
    int total = (locals_weight + 8 + 15) & ~15; // Round up weight + 8 to multiple of 16
    int align_size = total - 8; // Subtract push rbp
    if (align_size < 8) align_size = 8;


    return align_size;
}


void initWorkingContext()
{
    workingContext.typeStack[0] = COMPUTING_IMMEDIATE;
    workingContext.currentContextPtr = -1;
}

void pushWorkingContext(WCType type)
{
    workingContext.currentContextPtr++;
    workingContext.typeStack[workingContext.currentContextPtr] = type;
}

void popAllWorkingContexts()
{
    workingContext.currentContextPtr = -1;
}

void popWorkingContext()
{
    if (workingContext.currentContextPtr == -1) 
    {
        fprintf(stderr, "No working context to pop \n");
        exit(1);
    }

    workingContext.currentContextPtr--;
}


WCType getCurrentWorkingContext()
{
    return workingContext.typeStack[workingContext.currentContextPtr];
}

char *getLeaOrMovBasedOnContext()
{
    switch(getCurrentWorkingContext())
    {
        case COMPUTING_IMMEDIATE: 
        case COMPUTING_RVALUE: return "mov";
        case COMPUTING_LVALUE: return "lea";
    }
}

void initMatcher()
{
    matcher = malloc(sizeof(Matcher));
    

    matcher->expecations_to_match = malloc(MAX_EXPECTATIONS * sizeof(Expectation *));

    for (int i = 0; i < MAX_EXPECTATIONS; i++)
    {
        matcher->expecations_to_match[i] = malloc(sizeof(Expectation));
        matcher->expecations_to_match[i]->size = 0;
        matcher->expecations_to_match[i]->registerType = ANY;
    }
    matcher->current_expectation_index = 0;
}


void expectRegister(RegisterType registerType, int size)
{
    //printf("Expecting type: %i and size: %i. Index = %i \n", registerType, size, matcher->current_expectation_index);
    matcher->expecations_to_match[matcher->current_expectation_index]->registerType = registerType;
    matcher->expecations_to_match[matcher->current_expectation_index]->size = size;

    matcher->current_expectation_index++;
}

Expectation *getCurrentExpectation()
{
    if (matcher->current_expectation_index == 0) { return NULL; }

    return matcher->expecations_to_match[matcher->current_expectation_index - 1];
}

void clearExpectation()
{
    if (matcher->current_expectation_index == 0)
    {
        fprintf(stderr, "There is no expectation to clear.\n");
        exit(1);
    }
    matcher->current_expectation_index--;
    // printf("Cleared expecations. Index = %i \n", matcher->current_expectation_index);
}

void clearAllExpectations()
{
    matcher->current_expectation_index = 0;
}


void populateGPR(int index, char *name, int caller_saved, int callee_saved)
{
    // 64-bit register
    gpr->registers[index]->name = strdup(name);
    gpr->registers[index]->is_pinned = 0;
    gpr->registers[index]->type = GPR;
    gpr->registers[index]->size = 8;

    gpr->registers[index]->parentRegister = NULL;
    gpr->registers[index]->parent64Register = NULL;
    
    // 32-bit subregister (eax, ebx, etc.)
    char *eax_buffer = malloc(4);
    sprintf(eax_buffer, "e%cx", name[1]);  // "eax", "ebx", etc.
    gpr->registers[index]->subRegisters[0]->name = eax_buffer;
    gpr->registers[index]->subRegisters[0]->size = 4;
    gpr->registers[index]->subRegisters[0]->type = GPR;

    gpr->registers[index]->subRegisters[0]->is_pinned = 0;
    gpr->registers[index]->subRegisters[0]->parentRegister = gpr->registers[index];
    gpr->registers[index]->subRegisters[0]->parent64Register = gpr->registers[index];

    // 16-bit subregister (ax, bx, etc.)
    char *ax_buffer = malloc(3);
    sprintf(ax_buffer, "%cx", name[1]);    // "ax", "bx", etc.
    gpr->registers[index]->subRegisters[1]->name = ax_buffer;
    gpr->registers[index]->subRegisters[1]->size = 2;
    gpr->registers[index]->subRegisters[1]->type = GPR;

    gpr->registers[index]->subRegisters[1]->parentRegister = gpr->registers[index]->subRegisters[0];
    gpr->registers[index]->subRegisters[1]->parent64Register = gpr->registers[index];
    

    // 8-bit subregister (al, bl, etc.)
    char *al_buffer = malloc(3);
    sprintf(al_buffer, "%cl", name[1]);    // "al", "bl", etc.
    gpr->registers[index]->subRegisters[2]->name = al_buffer;
    gpr->registers[index]->subRegisters[2]->size = 1;
    gpr->registers[index]->subRegisters[2]->type = GPR;

    gpr->registers[index]->subRegisters[2]->is_pinned = 0;
    gpr->registers[index]->subRegisters[2]->parentRegister = gpr->registers[index]->subRegisters[1];
    gpr->registers[index]->subRegisters[2]->parent64Register = gpr->registers[index];
}

void init_GPR()
{
    gpr = malloc(sizeof(GPR));
    
    gpr->num_of_registers = 4;    
    gpr->registers = malloc(sizeof(Register*) * gpr->num_of_registers);


    for (int i = 0; i < gpr->num_of_registers; i++)
    {

        gpr->registers[i] = malloc(sizeof(Register));
        // 3 because there's only 3 subregisters within RAX,RBX, etc...
        for (int j = 0; j < 3; j++)
        {
            gpr->registers[i]->subRegisters[j] = malloc(sizeof(Register));
        }
    }

    populateGPR(0, "rax", 1, 0); // Caller-saved
    populateGPR(1, "rbx", 0, 1); // Callee-saved
    populateGPR(2, "rcx", 1, 0); // Caller-saved
    populateGPR(3, "rdx", 1, 0); // Caller-saved
}


Register *populateFPR(int index, char *name, int caller_saved, int callee_saved)
{

    // 64-bit register
    fpr->registers[index]->name = strdup(name);
    fpr->registers[index]->is_pinned = 0;
    fpr->registers[index]->size = 8;
    fpr->registers[index]->type = FPR;
    fpr->registers[index]->parentRegister = NULL;
    fpr->registers[index]->parent64Register = NULL;

    return fpr->registers[index];
}

void populateFPRSubRegisters(Register *parent64, char *name1, char *name2, char *name3)
{
    // 32-bit subregister (eax, ebx, etc.)
    parent64->subRegisters[0]->name = strdup(name1);
    parent64->subRegisters[0]->size = 4;
    parent64->subRegisters[0]->type = FPR;
    parent64->subRegisters[0]->is_pinned = 0;
    parent64->subRegisters[0]->parentRegister = parent64;
    parent64->subRegisters[0]->parent64Register = parent64;

    // 16-bit subregister (ax, bx, etc.)
    parent64->subRegisters[1]->name = strdup(name2);
    parent64->subRegisters[1]->size = 2;
    parent64->subRegisters[1]->type = FPR;
    parent64->subRegisters[1]->is_pinned = 0;
    parent64->subRegisters[1]->parentRegister = parent64->subRegisters[0];
    parent64->subRegisters[1]->parent64Register = parent64;
    

    // 8-bit subregister (al, bl, etc.)
    parent64->subRegisters[2]->name = strdup(name3);
    parent64->subRegisters[2]->size = 1;
    parent64->subRegisters[2]->type = FPR;
    parent64->subRegisters[2]->is_pinned = 0;
    parent64->subRegisters[2]->parentRegister = parent64->subRegisters[1];
    parent64->subRegisters[2]->parent64Register = parent64;
}


void init_FPR()
{
    fpr = malloc(sizeof(GPR));
    fpr->num_of_registers = 6;
    fpr->registers = malloc(sizeof(Register*) * fpr->num_of_registers);

    for (int i = 0; i < fpr->num_of_registers; i++)
    {
        fpr->registers[i] = malloc(sizeof(Register));

        for (int j = 0; j < 3; j++)
        {
            fpr->registers[i]->subRegisters[j] = malloc(sizeof(Register));
        }
    }
    

    // 1st argument 
    Register *rdi = populateFPR(0, "rdi", 1 , 0); //Caller-saved
    populateFPRSubRegisters(rdi, "edi", "di", "dil");
    
    // 2nd argument 
    Register *rsi = populateFPR(1, "rsi", 1 , 0); // Caller-saved
    populateFPRSubRegisters(rsi, "esi", "si", "sil");
    
    // 3rd argument 
    Register *rdx = populateFPR(2, "rdx", 1, 0); // Caller-saved
    populateFPRSubRegisters(rdx, "edx", "dx", "dl");
    
    // 4th argument 
    Register *rcx = populateFPR(3, "rcx", 1, 0); // Caller-saved
    populateFPRSubRegisters(rcx, "ecx", "cx", "cl");

    // 5th argument 
    Register *r8 = populateFPR(4, "r8", 1, 0); // Caller-saved
    populateFPRSubRegisters(r8, "r8d", "r8w", "r8b");

    // 6th argument 
    Register *r9 = populateFPR(5, "r9", 1, 0); // Caller-saved
    populateFPRSubRegisters(r9, "r9d", "r9w", "r9b");
    
}


char *getWordForSize(int size)
{
    switch (size)
    {
        case 1: return "byte";
        case 2: return "word";
        case 4: return "dword";
        case 8: return "qword";
    }

    fprintf(stderr, "Can't get word for size: %i \n", size);
    exit(1);
}



Register **getPinnedGPRs(int *in_count)
{


    Register **list = malloc(sizeof(Register*) * 10);
    int count = 0;

    for (int i = 0; i < gpr->num_of_registers; i++)
    {
        if (gpr->registers[i]->is_pinned)
        {
            list[count] = gpr->registers[i];
            count++;
        }
    }

    *in_count = count;
    if (count == 0) { return NULL; }

    list = realloc(list, sizeof(Register*)*count);
    return list;
}


void setRegisterPinState(Register *reg, int pin)
{
    if (pin) 
    {
        pinRegister(reg);
    }
    else
    {
        unpinRegister(reg);
    }


}

void clearRegisters(RegisterTable *table)
{
    for (int i = 0; i < table->num_of_registers; i++)
    {
        unpinRegister(table->registers[i]);
    }
}


/**
 * 
 * Gets a FPR register for the specified size. NEVER CALL DIRECTLY, ALWAYS THROUGH 'request'.
 */
Register *getFPR(int size)
{

    //printf("============================= \n");

    for (int i = 0; i < fpr->num_of_registers; i++)
    {
        //printf("Looking for FPR %s, checking if its free: %i \n", fpr->registers[i]->name, fpr->registers[i]->is_being_used);
        // Get a completely free register 
        if (fpr->registers[i]->is_pinned == 0) 
        {
            //printf("FPR register: %s is free, returning that one \n", fpr->registers[i]->name);
            switch (size)
            {
                case 8: { pinRegister(fpr->registers[i]); return fpr->registers[i]; }
                case 4: { pinRegister(fpr->registers[i]->subRegisters[0]); return fpr->registers[i]->subRegisters[0]; }
                case 2: { pinRegister(fpr->registers[i]->subRegisters[1]); return fpr->registers[i]->subRegisters[1]; }
                case 1: { pinRegister(fpr->registers[i]->subRegisters[2]); return fpr->registers[i]->subRegisters[2]; }
                default:
                {
                    fprintf(stderr, "Cannot get a FPR register for size: %i\n", size);
                    exit(1);
                }
            }
        
        }
    }

    fprintf(stderr, "Can't get FPR register for size: %i. All are being used\n", size);
    exit(1);
}

/**
 * 
 * Gets an UNPINNED (SPILLED OR NOT) GPR register for the specified size. NEVER CALL DIRECTLY, ALWAYS THROUGH 'request'.
 * 
 */
Register *getGPR(int size)
{
    
    for (int i = 0; i < gpr->num_of_registers; i++)
    {
        // Get a completely free register 
        if (gpr->registers[i]->is_pinned == 0) 
        {

            switch (size)
            {
                case 8: { pinRegister(gpr->registers[i]); return gpr->registers[i]; }
                case 4: { pinRegister(gpr->registers[i]->subRegisters[0]); return gpr->registers[i]->subRegisters[0]; }
                case 2: { pinRegister(gpr->registers[i]->subRegisters[1]); return gpr->registers[i]->subRegisters[1]; }
                case 1: { pinRegister(gpr->registers[i]->subRegisters[2]); return gpr->registers[i]->subRegisters[2]; }
                default:
                {
                    fprintf(stderr, "Cannot get a GPR register for size: %i\n", size);
                    exit(1);
                }
            }

        }
    }

    fprintf(stderr, "Can't get GPR register for size: %i. All are being used\n", size);
    exit(1);
}

/**
 * 
 * Gets an UNPINNED UNSPILLED GPR register for the specified size. NEVER CALL DIRECTLY, ALWAYS THROUGH 'request'.
 * 
 */
Register *getUnspilledGPR(int size)
{
    
    for (int i = 0; i < gpr->num_of_registers; i++)
    {
        // Get a completely free register 
        if (gpr->registers[i]->is_pinned == 0 && gpr->registers[i]->is_spilled == 0) 
        {

            switch (size)
            {
                case 8: { pinRegister(gpr->registers[i]); return gpr->registers[i]; }
                case 4: { pinRegister(gpr->registers[i]->subRegisters[0]); return gpr->registers[i]->subRegisters[0]; }
                case 2: { pinRegister(gpr->registers[i]->subRegisters[1]); return gpr->registers[i]->subRegisters[1]; }
                case 1: { pinRegister(gpr->registers[i]->subRegisters[2]); return gpr->registers[i]->subRegisters[2]; }
                default:
                {
                    fprintf(stderr, "Cannot get a GPR register for size: %i\n", size);
                    exit(1);
                }
            }

        }
    }

    fprintf(stderr, "Can't get GPR register for size: %i. All are being used\n", size);
    exit(1);
}



void push_bt_context(ContextType ctx_type)
{

    // Increment the block id and put the context on its right stack position
    increment_ctx_block_id(ctx_type);
    context.stack_index++;
    context.ctx_type_stack[context.stack_index] = ctx_type;

    // Increment the scope if applicable
    contextPushesScope(&context.current_scope);

}

void pop_bt_context()
{
    contextPopsScope(&context.current_scope);
    context.stack_index--;

}

int strToBinOPCode(char *str)
{
    if (strcmp(str, "+") == 0) {return ADD;}
    if (strcmp(str, "-") == 0) {return SUB;}
    if (strcmp(str, "*") == 0) {return MUL;}
    if (strcmp(str, "%") == 0) {return MOD;}
    if (strcmp(str, "/") == 0) {return DIV;}
    if (strcmp(str, ">") == 0) {return GT;}
    if (strcmp(str, "<") == 0) {return LT;}
    if (strcmp(str, ">=") == 0) {return GTEQ;}
    if (strcmp(str, "<=") == 0) {return LTEQ;}
    if (strcmp(str, "==") == 0) {return EQ;}
    if (strcmp(str, "!=") == 0) {return NEQ;}
    if (strcmp(str, "&") == 0)  { return BIT_AND; }
    if (strcmp(str, "|") == 0) { return BIT_OR; }
    if (strcmp(str, "&&") == 0) {return AND;}
    if (strcmp(str, "||") == 0) {return OR;}

    return UNKNOWN;
}

int strToUnaryOPCode(char *str)
{
    if (strcmp(str, "-") == 0) { return UNARY_MINUS; }
    if (strcmp(str, "!") == 0) { return UNARY_NOT; }
    if (strcmp(str, "&") == 0) { return UNARY_ADDRESS; }
    if (strcmp(str, "*") == 0) { return UNARY_DEREF; }
}

int strToPostfixOPCode(char *str)
{
    if (strcmp(str, "++") == 0) { return POSTFIX_INC; }
    if (strcmp(str, "--") == 0) { return POSTFIX_DEC; }

    fprintf(stderr, "Operation: '%s', is not recognized as a postfix operation \n", str);
    exit(1);
}

char *strToX86Binary(char *str)
{

    if (strcmp(str, "+") == 0) { return "add"; }
    if (strcmp(str, "*") == 0) { return "imul"; }
    if (strcmp(str, "-") == 0) { return "sub"; }

    return NULL;
}

char *strToX86Unary(char *str)
{
    if (strcmp(str, "-") == 0) { return "neg"; }
}

char *defineWordForSize(int size)
{
    switch(size)
    {
        case 1: return "db";
        case 2: return "dw";
        case 4: return "dd";
        case 8: return "dq";
    }

    fprintf(stderr, "No define word matches the given size: %i \n", size);
    exit(1);
}


int compute_binaryOperationDepth(ASTNode *node, int depth)
{
    //printf("Visiting node type %i at depth %i\n", node->node_type, depth);
    switch (node->node_type) 
    {
        case NODE_NULL:
        case NODE_NUMBER:
        case NODE_CHAR:
        case NODE_BOOL:
        case NODE_SUBSCRIPT:
        case NODE_FUNC_CALL:
        case NODE_IDENTIFIER: return depth;
        

        case NODE_POSTFIX_OP:
        case NODE_UNARY_OP: return depth + 1;



        case NODE_BINARY_OP:
        {
            printf("Got inside \n");
            int sub_left = compute_binaryOperationDepth(node->binary_op_node.left, depth + 1);
            int sub_right = compute_binaryOperationDepth(node->binary_op_node.right, depth + 1);
            return sub_left > sub_right ? sub_left : sub_right;
        }

        case NODE_PTR_FIELD_ACCESS:
        case NODE_FIELD_ACCESS:
        {
            return compute_binaryOperationDepth(node->field_access_node.base, depth + 1);
        }

        default:
        {
            fprintf(stderr, "Case: %i , not supported for computing bin op depth \n", node->node_type);
            exit(1);
        }
    }
}



char *write_loop_signature(FILE *asm_file, ASTNode *program)
{
    char *label = malloc(100 * sizeof(char));
    switch (program->node_type)
    {
        case NODE_WHILE:
        {
            emit_comment(asm_file, "While loop");
            fprintf(asm_file, ".while_loop_%i:\n", context.while_block_id);
            sprintf(label, ".while_loop_%i", context.while_block_id);
            break;
        }

        case NODE_FOR:
        {
            emit_comment(asm_file, "For loop");
            fprintf(asm_file,".for_loop_%i:\n", context.for_block_id);
            sprintf(label, ".for_loop_%i", context.for_block_id);
            break;
        }

        case NODE_FOREACH:
        {
            emit_comment(asm_file, "Foreach loop");
            fprintf(asm_file, ".foreach_loop_%i:\n", context.foreach_block_id);
            sprintf(label, ".foreach_loop_%i", context.foreach_block_id);
            break;
        }
    }

    return label;
}


char *write_if_signature(FILE *asm_file)
{
    char *label = malloc(100 * sizeof(char));
    
    emit_comment(asm_file, "If statement");
    fprintf(asm_file, ".if_stmt_%i:\n", context.if_block_id);
    sprintf(label, ".if_stmt_%i", context.if_block_id);

    return label;

}



void collectStrings(FILE *asm_file, ASTNode *program)
{
    if (program == NULL)
    {
        return;
    }

    switch(program->node_type)
    {
        case NODE_BLOCK:
        {
            for (int i = 0; i < program->block_node.statement_count; i++)
            {
               collectStrings(asm_file, program->block_node.statements[i]);
            }
            break;
        }

        case NODE_USE_DIRECTIVE:
        {
            collectStrings(asm_file, program->use_node.program);
            break;
        }

        case NODE_FUNC_DEF:
        {
            collectStrings(asm_file, program->funcdef_node.body);
            break;
        }

        case NODE_STDALONE_FUNC_CALL:
        {
            for (int i = 0; i < program->stdalone_func_call_node.params_count; i++)
            {
                collectStrings(asm_file, program->stdalone_func_call_node.params_expr[i]);
            }
            break;
        }

        case NODE_FUNC_CALL:
        {
            for (int i = 0; i < program->funccall_node.params_count;i++)
            {
                collectStrings(asm_file, program->funccall_node.params_expr[i]);
            }
            break;
        }

        case NODE_ASSIGNMENT:
        {
            collectStrings(asm_file, program->assignment_node.expression);
            break;
        }

        case NODE_REASSIGNMENT:
        {
            // collect the lvalue too? Can there be strings there?

            // collect the rvalue
            collectStrings(asm_file, program->reassignment_node.expression);
            break;
        }

        case NODE_FOR:
        {
            collectStrings(asm_file, program->for_node.body);
            break;
        }

        case NODE_WHILE:
        {
            collectStrings(asm_file, program->while_node.body);
            break;
        }

        case NODE_FOREACH:
        {
            collectStrings(asm_file, program->foreach_node.body);
            break;
        }

        case NODE_IF:
        {
            collectStrings(asm_file, program->if_node.body);
            if (program->if_node.else_body != NULL)
            {
                collectStrings(asm_file, program->if_node.else_body);
            }
            break;
        }

        case NODE_ARRAY_INIT:
        {
            for (int i = 0; i < program->array_init_node.size; i++)
            {
                collectStrings(asm_file, program->array_init_node.elements[i]);
            }
            break;
        }


        case NODE_RETURN:
        {
            collectStrings(asm_file, program->return_node.return_expr);
            break;
        }

        case NODE_STR:
        {
            // Print the label and string value separately because if the string has special chars we 
            // need to convert that to uint
            fprintf(asm_file, "%s: db ", program->str_node.label);

            write_string(asm_file, program->str_node.str_value);
            break;
        }

        default:
        {
            break;
        }
    }
}


void buildInnerFunc(FILE *asm_file, ASTNode *program, SymbolTable *st, FunctionTable *ft)
{
    switch (program->node_type)
    {
        case NODE_BLOCK:
        {
            int i = 0;
            int sc = program->block_node.statement_count;
            ASTNode *current = NULL;
            while (i < sc)
            {
                current = program->block_node.statements[i];
                if (current->node_type == NODE_FUNC_DEF)
                {
                    buildStart(asm_file, current, st, ft);
                }

                i++;
            }

            return;
        }

        default:
        {
            return;
        }
    }
}


// There is no way of building the data for an assignment and then building it again 
// in buildStart(), because to get to that assignment we need to traverse the AST Tree through
// a function node, which we dont handle in buildData()

// Handle > 8 bytes global declaration
void buildData(FILE *asm_file, ASTNode *program, SymbolTable *current_st, FunctionTable *current_ft)
{
    switch(program->node_type)
    {
        case NODE_BLOCK:
        {
            for (int i = 0; i < program->block_node.statement_count; i++)
            {
               buildData(asm_file, program->block_node.statements[i], current_st, current_ft); 
            }
            break;

        }

        case NODE_USE_DIRECTIVE:
        {
            buildData(asm_file, program->use_node.program, current_st, current_ft);
            break;
        }

        case NODE_CAST:
        {
            buildData(asm_file, program->cast_node.expr, current_st, current_ft);
            break;
        }

        case NODE_ENUM:
        {
            uint8_t count = program->enum_node.declaration_count;

            for (uint8_t i = 0; i < count; i++)
            {
                fprintf(asm_file, "%s db %i\n", program->enum_node.declarations[i]->declaration_node.identifier, i);
            }

            break;
        }

        case NODE_FUNC_DEF:
        {


            // For proper function resolution under right scope
            push_bt_context(CTX_FUNCTION);

            // Mark the node as built


            char **param_types = get_paramTypes_from_Params(program->funcdef_node.params, program->funcdef_node.params_count);

            //printf("HERE \n");
            Function *f = lookup_function(current_ft, context.current_scope, 
                                            program->funcdef_node.func_name, param_types, program->funcdef_node.params_count);
        
            
            //printf("WE FOUND f: %s, getting inside\n", f->identifier);
            buildData(asm_file, program->funcdef_node.body, f->local_symbols, f->local_functions);
            //printf("GOT OUT\n");

            pop_bt_context();

            break;
        }

        
        case NODE_ASSIGNMENT:
        {
            int size = size_of_type(program->assignment_node.type);
            



            // Try find the identifier in the global scope
            Symbol *s = check_strict_identifier(current_st, 0, program->assignment_node.identifier);


            if (s == NULL) { return; }

            char *define_enough_bytes = defineWordForSize(size_of_type(program->assignment_node.type));

            fprintf(asm_file, "%s %s ", s->identifier, define_enough_bytes);
            buildData(asm_file, program->assignment_node.expression, current_st, current_ft);            

            break;
        }


        // 27/11 - What happens if we do global str s; ?? dunno 
        // gotta look into that
        case NODE_DECLARATION:
        {
            int size = size_of_type(program->declaration_node.type);
             

            Symbol *s = check_identifier(current_st, 0, program->declaration_node.identifier);

            if (s == NULL) { return; }

            char *define_enough_bytes = defineWordForSize(size);

            fprintf(asm_file, "%s %s 0\n", program->declaration_node.identifier, define_enough_bytes);
            break;
        }


        case NODE_NUMBER:
        {

            fprintf(asm_file, "%"PRId64"\n", program->number_node.number_value);
            break;
        }

        case NODE_CHAR:
        {
            fprintf(asm_file, "%i\n", program->char_node.char_value);
            break;
        }

        case NODE_STR:
        {
            // already collected
            break;
        }

        case NODE_BOOL:
        {
            char c = (strcmp(program->bool_node.bool_value,"true") == 0) ? 1 : 0;

            fprintf(asm_file, "%i\n", c);
            break;
        }
        

        default:
        {
            break;
        }
    }


}

Operand buildExtern(FILE *asm_file, ASTNode *program, FunctionTable *current_ft)
{
    switch(program->node_type)
    {
        case NODE_BLOCK:
        {
            for (int i = 0; i < program->block_node.statement_count; i++)
            {
               buildExtern(asm_file, program->block_node.statements[i], current_ft); 
            }
            
            Operand none;
            return none;
        }

        case NODE_USE_DIRECTIVE:
        {
            buildExtern(asm_file, program->use_node.program, current_ft);
            Operand none;
            return none;
        }

        case NODE_EXTERN_FUNC_DEF:
        {
            char **param_types = get_paramTypes_from_Params(program->extern_func_def_node.params, program->extern_func_def_node.params_count);
            Function *extern_function = lookup_function(gb_functionTable,0,program->extern_func_def_node.func_name,param_types, program->extern_func_def_node.params_count);
            emit_extern_function(asm_file, extern_function->signature);
            Operand none;
            return none;
        }

        default:
            break;


    }
}

Operand buildStart(FILE *asm_file, ASTNode *program, SymbolTable *current_st, FunctionTable *current_ft)
{
    if (program == NULL)
    {
        fprintf(stderr, "Trying to build for a NULL node \n");
        exit(1);
    }

    //printf("Building for node: %s, (%i) \n", astTypeToStr(program), program->node_type);

    switch (program->node_type) 
    {   
        case NODE_BLOCK:
        {
            enter_node(program);
            for (int i = 0; i < program->block_node.statement_count; i++)
            {
               buildStart(asm_file, program->block_node.statements[i], current_st, current_ft); 
            }
            
            exit_node(program);
            Operand none;
            return none;
        }

        case NODE_USE_DIRECTIVE:
        {
            enter_node(program);

            buildStart(asm_file, program->use_node.program, current_st, current_ft);

            exit_node(program);
            Operand none;
            return none;
        }

        case NODE_CAST:
        {
            enter_node(program);
            Operand casting_expr = buildStart(asm_file, program->cast_node.expr, current_st, current_ft); 
            exit_node(program);
            
            return casting_expr;
        }

        case NODE_WHILE:
        {
            enter_node(program);
            // Write the while loop label
            char *label = write_loop_signature(asm_file, program);
            char *end = malloc(120 * sizeof(char));
            sprintf(end, ".end_%s", label);

            // Condition check
            emit_comment(asm_file, "While loop condition check");
            Operand condition = buildStart(asm_file, program->while_node.condition_expr, current_st, current_ft);
            
            // Compare with 0. Use the value sitting in condition.linkedToRegister
            fprintf(asm_file, "    cmp %s, 0\n", condition.linkedToRegister->name);

            // We can unpin now, the condition has already been checked
            // Clear the registers, nothing is longer needed
            unpinRegister(condition.linkedToRegister);
            clearRegisters(gpr);

            // Jump to end while label if condition false
            fprintf(asm_file, "    je %s\n",end);

            // While body
            emit_comment(asm_file, "Body");
            buildStart(asm_file, program->while_node.body, current_st, current_ft);

            // Back to while loop condition check
            emit_comment(asm_file, "Back to while loop condition check");
            fprintf(asm_file, "    jmp %s\n\n", label);

            // End while loop label
            fprintf(asm_file, "%s:\n", end);
            
            exit_node(program);
            break;
        }

       
        case NODE_FOREACH:
        {
            enter_node(program);

            emit_comment(asm_file, "Lowered foreach node");
            buildStart(asm_file, program->foreach_node.foreach_lowered, current_st, current_ft);
            emit_comment(asm_file, "End lowered foreach node");

            exit_node(program);
            break;
        }


        case NODE_FOR:
        {
            enter_node(program);

            // Initial assignment of for loop iterator
            emit_comment(asm_file, "Initial assignment of for loop iterator");
            buildStart(asm_file, program->for_node.assignment_expr, current_st, current_ft);

            // Write the for loop label
            char *label = write_loop_signature(asm_file, program);
            char *end = malloc(120 * sizeof(char));
            sprintf(end, ".end_%s", label);
            
            // Condition check 
            emit_comment(asm_file, "Condition check of for loop");
            Operand condition = buildStart(asm_file, program->for_node.condition_expr, current_st, current_ft);

            // Compare with 0. Use the value sitting in condition.linkedToRegister
            fprintf(asm_file, "    cmp %s, 0\n", condition.linkedToRegister->name);

            // We can unpin now, the condition has already been checked
            // Clear the registers, nothing is longer needed
            unpinRegister(condition.linkedToRegister);
            clearRegisters(gpr);

            // Jump to end for label if condition false
            fprintf(asm_file, "    je %s\n", end);

            // Body
            emit_comment(asm_file, "Body");
            buildStart(asm_file, program->for_node.body, current_st, current_ft);

            // Reassignment of the iterator
            emit_comment(asm_file, "Reassignment of the for loop iterator");
            buildStart(asm_file, program->for_node.reassignment_expr, current_st, current_ft);

            // Back to condition check 
            emit_comment(asm_file, "Back to condition check of for loop");
            fprintf(asm_file,"    jmp %s\n\n", label);

            // End for loop label
            fprintf(asm_file,"%s:\n", end);
            
            exit_node(program);
            break;
        }

        case NODE_IF: 
        {
            enter_node(program);
            
            if (program->if_node.else_body != NULL) {  increment_ctx_block_id(CTX_ELSE); }

            // Write if signature and create possible else signature
            char *if_label = write_if_signature(asm_file);
            char *else_label = malloc(120 * sizeof(char));
            sprintf(else_label, ".else_stmt_%i", context.else_block_id);
            char *end = malloc(120 * sizeof(char));
            sprintf(end, ".end_%s", if_label);
            
            
            // Condition 
            emit_comment(asm_file, "Condition check of if block");
            Operand condition = buildStart(asm_file, program->if_node.condition_expr, current_st, current_ft);
            
            // Compare with 0. Use the value sitting in condition.linkedToRegister
            fprintf(asm_file, "    cmp %s, 0\n", condition.linkedToRegister->name);

            // We can unpin now, the condition has been checked
            // Clear the registers. Nothing is longer needed
            unpinRegister(condition.linkedToRegister);
            clearRegisters(gpr);

            // Jump if condition false to else body if it's not null
            if (program->if_node.else_body != NULL)
            {

                fprintf(asm_file, "    je %s\n", else_label);
            }
            // Jump if condition false to end if block
            else 
            {

                fprintf(asm_file,"    je %s\n", end); 
            }
            
            // If Body 
            buildStart(asm_file, program->if_node.body, current_st, current_ft);
            // Jump to end if label, don't fall onto the else body block
            fprintf(asm_file, "    jmp %s\n", end);

            // Clear the registers after the if body. No register is longer needed
            clearRegisters(gpr);


            // Else Body 
            if (program->if_node.else_body != NULL)
            {
                push_bt_context(CTX_ELSE);
                fprintf(asm_file, "%s:\n", else_label);
                buildStart(asm_file, program->if_node.else_body, current_st, current_ft);
                // Clear the registers after the else body. No register is longer needed
                clearRegisters(gpr);
                pop_bt_context();
            }
            
            fprintf(asm_file, "%s:\n", end);
            exit_node(program);
            break; 
        }


        case NODE_FUNC_DEF:
        {
            enter_node(program);

            if (program->funcdef_node.already_built) { break; }



    
            // Get the defined function
            char **param_types = get_paramTypes_from_Params(program->funcdef_node.params, program->funcdef_node.params_count);

            Function *f = lookup_function(current_ft, context.current_scope, 
                                          program->funcdef_node.func_name, param_types, program->funcdef_node.params_count);

        
            // Mark the AST as visited
            program->funcdef_node.already_built = 1;
            
            // Build the inner functions first
            buildInnerFunc(asm_file, program->funcdef_node.body, f->local_symbols, f->local_functions);

            


            
            // Update stack pointer and current function
            updateCFF(f);
            

            // Write the function signature, the label
            emit_label(asm_file, f->signature);


            // Emit prologue
            emit_prologue(asm_file);

            

            // Start tracking rsp
            start_tracking_rsp();
            // Load the parameters 
            emit_comment(asm_file, "Loading parameters");
            int params_to_it_until = min(6, f->param_count);
            for (int i = 0; i < params_to_it_until; i++)
            {
                Symbol *param = get_ParamAtIndex(f->local_symbols, i);

                // I think the word is irrelevant
                //char *word = getWordForSize(param->size);

                // Where the parameters comes from
                Operand op_src;
                op_src.is_unsigned = is_type_unsigned_int(param->type);
                op_src.linkedToRegister = request_specific_size_and_type_reg(FPR, param->size);
                
                emit_mov_r2s(asm_file, &op_src, getStackBasePtr() - param->offset, param->size);

            }

            // If we have more than 6 params, calculate their stack position and place them
            // at the correct current stack frame position

            // If the param count is odd, there is an extra 8 bytes we have to consider
            // coming from the sub rsp,8 that the caller executes to ensure 16 byte alignment


            int is_param_count_odd = f->param_count % 2 != 0 ? 1 : 0;
            if (f->param_count > 6)
            {
                for (int i = f->param_count - 1; i >= 6; i--)
                {
                    Symbol *p = get_ParamAtIndex(f->local_symbols, i);
                    Register *temp = request_specific_size_and_type_reg(GPR, p->size);
                    int argument_offset = 8 + 8 * is_param_count_odd + 8 * (f->param_count - i);
                    fprintf(asm_file, "    mov %s, [rbp + %i]     ; %i th argument\n", temp->name, argument_offset, i + 1); // rbp + (padding and weight) + call + 8 * (N - 6)
                    fprintf(asm_file, "    mov %s [rbp - %i], %s \n", getWordForSize(temp->size),
                            getStackBasePtr() - p->offset, temp->name);
                    unpinRegister(temp);
                }
            }

            emit_comment(asm_file, "Done loading parameters");

             
            // Instructions 
            buildStart(asm_file, program->funcdef_node.body, f->local_symbols, f->local_functions ); 


            // Return label
            emit_label(asm_file, f->ret_signature);

            // Epilogue
            emit_epilogue(asm_file);

            // Stop tracking rsp
            stop_tracking_rsp();

            exit_node(program);
            break;
        }
        
        /**
         * NOT CHANGED
         */
        case NODE_STDALONE_FUNC_CALL:
        case NODE_FUNC_CALL:
        {
            enter_node(program);

            // Get the function we are calling 
            Function *f = lookup_function(current_ft, context.current_scope, 
                                          program->funccall_node.identifier, program->funccall_node.params_type, program->funccall_node.params_count);



                                        
            emit_comment(asm_file, "Preparing to call function");

            // Start tracking rsp
            start_tracking_rsp();

            // Save caller-saved registers, push
            // Check if any is being used
            // Keep track of the number of push reg to ensure rsp alignment
            int count = 0;
            Register **used_registers = getPinnedGPRs(&count);
            int is_rax_busy = RAX->is_pinned;
            
            /**     ----------------   SAVING REGISTERS THAT HOLD LIVE VALUES BEFORE BEING OVERWRITTEN BY THE NEXT CALL --------------- */
            if (used_registers != NULL)
            {
                emit_comment(asm_file, "Saving used registers");
                for (int i = 0; i < count; i++)
                {
                    spill_reg(asm_file, used_registers[i]);
                }
            }

            /**     ----------------   END SAVING REGISTERS THAT HOLD LIVE VALUES BEFORE BEING OVERWRITTEN BY THE NEXT CALL --------------- */


            /**    ---------------  COMPUTING THE ARGUMENTS AND PUSHING TO THE STACK ARGUMENTS WHEN WE HAVE MORE THAN 6 --------------------------------- */
            Operand par;

            int args_pushed = 0;


            
            // Build the expressions for the parameters 
            for (int i = 0; i < program->funccall_node.params_count; i++)
            {
                //fprintf(asm_file, "    ;%i-th argument\n",i + 1);
                 // Push to the stack if >6 parameters
                if (i >= 6)
                {

                    expectRegister(GPR, size_of_type(program->funccall_node.params_type[i]));
                    par = buildStart(asm_file, program->funccall_node.params_expr[i], current_st, current_ft);
                    clearExpectation();
                    
                    //Ensure we push the 8 byte parent 
                    par.linkedToRegister = getFamilyRegWithSize(par.linkedToRegister, 8);
                    emit_push(asm_file, par.linkedToRegister->name);
                    args_pushed++;
                    unpinRegister(par.linkedToRegister);
                    continue;
                }

                expectRegister(ANY, size_of_type(program->funccall_node.params_type[i]));
                par = buildStart(asm_file, program->funccall_node.params_expr[i], current_st, current_ft);
                clearExpectation();


                //Ensure the parameters are stored in the correct FPR registers, otherwise move it in
                if (par.linkedToRegister->type != FPR)
                {
                    
                    Register *fpr_reg = getFamilyRegWithSize(fpr->registers[i], size_of_type(program->funccall_node.params_type[i]));
                    Operand op_dst = {.is_unsigned = par.is_unsigned, .linkedToRegister = fpr_reg};

                    fprintf(asm_file, "    ;Changing to FPR, par was stored in GPR: %s\n", par.linkedToRegister->name);
                    emit_mov_r2r(asm_file, &op_dst, &par, fpr_reg->size);
                }

                // Unpin the register used to store the parameter, its now in the right FPR
                unpinRegister(par.linkedToRegister);

            }

            
            /**    ---------------  END COMPUTING THE ARGUMENTS AND PUSHING TO THE STACK ARGUMENTS WHEN WE HAVE MORE THAN 6 --------------------------------- */


            // Before the call, ensure 16 byte alignment rsp 
            emit_ensure16byte_rsp(asm_file);


            // call func_label
            emit_call(asm_file, f->signature);
            

            Operand return_value;
            
            // If return type different than void, there IS a returned value in rax
            if (strcmp(f->rt_type, "void") != 0)
            {
                int size_of_return = size_of_type(f->rt_type);
                Operand rax_operand = {.is_unsigned = is_type_unsigned_int(f->rt_type), .linkedToRegister = getFamilyRegWithSize(RAX, size_of_return)};

                // If RAX wasnt busy before, thats fine, we are not overwriting nothing
                if (!is_rax_busy)
                {
                    return_value = rax_operand;
                }

                // If RAX was busy before, move the return value somewhere else
                else 
                {
                    emit_comment(asm_file, "RAX is busy, move the returned value somewhere else AND THAT HAS NOT BEEN PUSHED");

                    return_value.linkedToRegister = request_unspilled_specific_size_reg(size_of_return);

                    return_value.is_unsigned = is_type_unsigned_int(f->rt_type);


                    emit_mov_r2r(asm_file, &return_value, &rax_operand, size_of_return);
                }

                pinRegister(return_value.linkedToRegister);

            }




            // After the call, restore the rsp. Undoes what 'emit_ensure16byte_rsp() did' so:
            emit_restore_rsp(asm_file, args_pushed);

            // collect the spilled registers
            collect_spilled_regs(asm_file);
            
            // Stop tracking RSP
            stop_tracking_rsp();


            emit_comment(asm_file, "Ended func call node");

            // Exit
            exit_node(program);


            //printf("FINSIDH THE FUNC CALL \n");

            return return_value;
        }

        case NODE_RETURN:
        {
            enter_node(program);
            emit_comment(asm_file, "Return node");
            Operand ret;

            // Get the return label
            char *ret_label = getCurrentFunction()->ret_signature;
            if (program->return_node.return_expr == NULL)
            {
                fprintf(asm_file, "    jmp %s\n", ret_label);
                exit_node(program);
                return ret;
            }
            

            // Build the return expresion
            ret = buildStart(asm_file, program->return_node.return_expr, current_st, current_ft);          

            // If return is not already in RAX's family, move it there 
            if (!are_reg_family(ret.linkedToRegister, RAX))
            {

                emit_comment(asm_file, "Ensure return is in rax");
                // Request access to RAX, spill to stack if needed
                // Truncate to match the size of the return type
                Register *rax = request_specific_reg(asm_file, RAX);
                Register *sized_rax = getFamilyRegWithSize(rax, ret.linkedToRegister->size);

                // Move the register associated to the return value to RAX's family
                // Unpin the register that held the return value before
                // Link the register associated to the return value with sized_rax

                Operand op_dst = {.linkedToRegister = sized_rax, .is_unsigned = ret.is_unsigned};
                Operand op_src = {.linkedToRegister = ret.linkedToRegister, .is_unsigned = ret.is_unsigned};

                emit_mov_r2r(asm_file, &op_dst, &op_src, op_src.linkedToRegister->size);

                unpinRegister(ret.linkedToRegister);
                ret.linkedToRegister = sized_rax;
            }

            // Pin the register associated to the return value
            pinRegister(ret.linkedToRegister);

            // Jump to return label
            fprintf(asm_file, "    jmp %s\n", ret_label);
            exit_node(program);
            emit_comment(asm_file, "Ended return node");
            return ret;
        }
        
        
        case NODE_ARRAY_INIT:
        {
            enter_node(program);

            emit_comment(asm_file, "Array init");
            // Get the expected register 
            Symbol *arr = lookup_identifier(current_st, context.current_scope, program->array_init_node.arr_name);
            char *element_type = get_arr_element_type(arr->type);
            int size_of_every_element = size_of_type(element_type); 

            Register *first_element = NULL;
            
            /**
             * 28/11: Later change to > 16. Remember to use XMM registers to mov < 16 byte data
             */
            if (size_of_every_element > 8)
            {
                for (int i = 0; i < program->array_init_node.size; i++)
                {

                    emit_comment(asm_file, "Copying");

                    fprintf(asm_file, "    lea rdi, [rbp - %i]\n", getStackBasePtr() - arr->offset);
                    fprintf(asm_file, "    add rdi, %i \n", i * size_of_every_element);  

                    // lea rax, [rbp - offset] -> loads into rax the address of ith element
                    Operand result = with_lvalue_context(asm_file, program->array_init_node.elements[i], current_st, current_ft); 

                    fprintf(asm_file, "    lea rsi, [%s]\n", result.linkedToRegister->name);
                    fprintf(asm_file, "    mov rcx, %i \n", size_of_every_element);
                    fprintf(asm_file, "    rep movsb \n");

                    // Link a register to the base of the array (first element)
                    if (i == 0)  { first_element = result.linkedToRegister; }

                    // Unpin the register, it's already been consumed
                    unpinRegister(result.linkedToRegister);
                }
            }

            // normal mov
            else 
            {
                // Push an expectation, every element of the array must use the same size registers
                expectRegister(GPR, size_of_every_element);
                char *byte_word = getWordForSize(size_of_every_element);

                for (int i = 0; i < program->array_init_node.size; i++)
                {
                    Operand result = buildStart(asm_file, program->array_init_node.elements[i], current_st, current_ft);

                    emit_mov_r2s(asm_file, &result, getStackBasePtr() - (i * size_of_every_element + arr->offset), size_of_every_element);
                    if (i == 0) 
                    {
                        first_element = result.linkedToRegister;
                    }
                    // Unpin the register, it's already been consumed
                    unpinRegister(result.linkedToRegister);
                }

                // Clear the expectation, we are finished
                clearExpectation();
            }

            
            // Pin the register holding the base address of the array
            pinRegister(first_element);

            emit_comment(asm_file, "Array init end");
            exit_node(program);
            Operand result = {.linkedToRegister = first_element};
            return result;
        }

        case NODE_SUBSCRIPT: 
        {

            enter_node(program);

            emit_comment(asm_file, "Array subscript");



            Operand base;

            
            base = with_lvalue_context(asm_file, program->subscript_node.base, current_st, current_ft);

            if (is_type_str(program->subscript_node.base_type))
            {
                emit_comment(asm_file, "String subscript");
                emit_mov_a2r(asm_file, &base, &base, 8); // mov rax, [rax]
            }

            /**
             * Compute the RVALUE (value) index on an 8 byte register
             */
            emit_comment(asm_file, "Computing the index");
            Operand index = with_rvalue_context(asm_file, program->subscript_node.index, ANY, program->subscript_node.index_size, current_st, current_ft);
            

            Operand op_index_parent = { .linkedToRegister = getParent64(index.linkedToRegister) };


            if (index.linkedToRegister->size < 4)
            {
                
                emit_mov_r2r(asm_file, &op_index_parent, &index, 8);
            }

            index.linkedToRegister = op_index_parent.linkedToRegister;


            /**
             * Compute the position of the element to retrieve.
             * Multiply the index by the element size. Specify the bytes we are doing operation with, in this case 8 bytes
             */
            emit_comment(asm_file, "Computing the position of the element to retrieve");
            emit_mul_imm(asm_file, index, program->subscript_node.element_size);


            /**
             * 
             * Add the index offset to the base address
             */
            fprintf(asm_file, "    add %s, %s \n", base.linkedToRegister->name, index.linkedToRegister->name);
        
            // Unpin the register holding the index value, it's been consumed
            unpinRegister(index.linkedToRegister);

            // If we are in LVALUE context, we want the address
            Operand op_result = {.linkedToRegister = base.linkedToRegister, .is_unsigned = base.is_unsigned};

            // If computing RVALUE, we want the value, not the address 
            if (getCurrentWorkingContext() == COMPUTING_RVALUE || getCurrentWorkingContext() == COMPUTING_IMMEDIATE) 
            {

                // Move into it the value sitting at the address we calculated
                emit_mov_a2r(asm_file, &op_result, &base, program->subscript_node.element_size);

                // Unpin the register holding the address
                unpinRegister(base.linkedToRegister);

                // Make sure op_result is of the size specified
                op_result.linkedToRegister = getFamilyRegWithSize(op_result.linkedToRegister, program->subscript_node.element_size);
            }

            // Pin the register holding the result, whether is the address or the value
            pinRegister(op_result.linkedToRegister);

            emit_comment(asm_file, "Array subscript end");
            //printf("ARRAY SUBSCRIPT END RESULT = %s\n", op_result.linkedToRegister->name);
            exit_node(program);

            return op_result;
           
        }

        case NODE_FIELD_ACCESS:
        {
            enter_node(program);
            emit_comment(asm_file, "Field access");

            /**
             * Retrieve the type the instance is of
             */
            Object *o = lookup_object(gb_objectTable, program->field_access_node.type);

            /**
             * Get the field we are trying to access so we know its offset
             */
            Symbol *field = lookup_identifier(o->local_symbols, 1, program->field_access_node.field_name);

            /**
             * Compute the LVALUE (address) base
             */
            Operand base = with_lvalue_context(asm_file, program->field_access_node.base, current_st, current_ft);

            /**
             * Add the field offset to the base
             */
            fprintf(asm_file, "    add %s, %i \n", base.linkedToRegister->name, field->offset);   

            // If we are in LVALUE context, we want the address
            Operand op_result = {.is_unsigned = base.is_unsigned, .linkedToRegister = base.linkedToRegister};

            // If computing RVALUE, we want the value, not the address 
            if (getCurrentWorkingContext() == COMPUTING_RVALUE || getCurrentWorkingContext() == COMPUTING_IMMEDIATE)
            {
                // Request a GPR register big enough to hold the field data type
                op_result.linkedToRegister = request_specific_size_and_type_reg(GPR, field->size);

                // Move into it the value sitting at the address we calculated
                emit_mov_a2r(asm_file, &op_result, &base, field->size);

                // Unpin the register holding the address, is no longer needed
                unpinRegister(base.linkedToRegister);
            }

            // Pin the register holding the result, whether is the address or the value
            pinRegister(op_result.linkedToRegister);

            emit_comment(asm_file, "Field access end");
            exit_node(program);

            return op_result;
        }

        case NODE_PTR_FIELD_ACCESS:
        {
            enter_node(program);
            emit_comment(asm_file, "Ptr field access");

            /**
             * Calculate the base address (pointer)
             * Dereference so we get the address of the pointed object
             */
            Operand base = with_lvalue_context(asm_file, program->ptr_field_access_node.base, current_st, current_ft);
            emit_mov_a2r(asm_file, &base, &base, base.linkedToRegister->size);

                    

            /**
             * Get the pointed type so we can retrieve the object
             */
            char *pointed_type = get_pointed_type(program->ptr_field_access_node.type, 1);

            /**
             * Retrieve the built-in type the instance is of
             */
            Object *o = lookup_object(gb_objectTable, pointed_type);

            /**
             * Get the field we are trying to access so we know its offset
             */
            Symbol *field = lookup_identifier(o->local_symbols, 1, program->ptr_field_access_node.field_name);



            /**
             * Add the field offset to the base
             */
            fprintf(asm_file, "    add %s, %i \n", base.linkedToRegister->name, field->offset);


            // If we are in LVALUE context, we want the address
            Operand op_result = {.is_unsigned = base.is_unsigned, .linkedToRegister = base.linkedToRegister};
            // If computing RVALUE, we want the value, not the address 
            if (getCurrentWorkingContext() == COMPUTING_RVALUE || getCurrentWorkingContext() == COMPUTING_IMMEDIATE)
            {            

                // Move into it the value sitting at the address we calculated
                emit_mov_a2r(asm_file, &base, &base, field->size);

                // Unpin the register holding the address, no longer needed
                unpinRegister(base.linkedToRegister);

                // Make sure op_result is of the size specified
                op_result.linkedToRegister = getFamilyRegWithSize(op_result.linkedToRegister, field->size);
            }

            // Pin the register holding the result, whether is the address or the value
            pinRegister(op_result.linkedToRegister);


            emit_comment(asm_file, "Ptr field access end");
            exit_node(program);


            return op_result;
        }

        case NODE_ASSIGNMENT:
        {

            enter_node(program);

            Operand none;

            /**
             * Do nothing if the Context is NONE. This means the assignment is for a global variable
             * which was done in buildData(). Dont build again for it
             */
            if (get_current_context() == CTX_NONE)
            {
                return none;
            }
            

            int size = size_of_type(program->assignment_node.type);


            fprintf(asm_file, "    ;Building assignment expression \n");

            Operand rvalue = with_rvalue_context(asm_file, program->assignment_node.expression, GPR, size, current_st, current_ft );

            Symbol *identifier = lookup_identifier(current_st, context.current_scope, program->assignment_node.identifier);

            

            /**
             * If the identifier is not of type array, then we must move it onto its right position in the stack.
             * For an array type, this was done in NODE_ARRAY_INIT
             */
            if (!is_type_array(identifier->type))
            {
                emit_mov_r2s(asm_file, &rvalue, getStackBasePtr() - identifier->offset, identifier->size);
            }
           
            // Unpin rvalue, its no longer needed 
            unpinRegister(rvalue.linkedToRegister);

            exit_node(program);
            return none;
        }

        case NODE_REASSIGNMENT:
        {

            enter_node(program);

            Operand none;

            if (get_current_context() == CTX_NONE)
            {
                return none;
            }

            Operand lvalue;
            Operand rvalue;

            emit_comment(asm_file, "Reassignment");

            emit_reassign(asm_file, &lvalue, &rvalue, &program->reassignment_node, current_st, current_ft);

            exit_node(program);
            
            return none;
        }

        case NODE_DECLARATION:
        {
            enter_node(program);
            Operand none;
            // Done in the first pass 
            if (get_current_context() == CTX_NONE)
            {
                return none;
            }

            exit_node(program);
            return none;
        }

        /**
         * NOT CHANGED
         */
        case NODE_BINARY_OP:
        {
            

            enter_node(program);

            /**
             * Compute the binary operation depth. The deepest branch will be evaluated first so we dont run out of registers
             */
            int left_depth = compute_binaryOperationDepth(program->binary_op_node.left, 1);
            int right_depth = compute_binaryOperationDepth(program->binary_op_node.right, 1);
            
            
            Register *temp = NULL;

            Operand left;
            Operand right;

            /**
             * Build for the deepest branch of the binary operation first
             */
            emit_binop_deepest(asm_file, &program->binary_op_node, &left, &right, current_st, current_ft);

            BinOPCode binOperation = strToBinOPCode(program->binary_op_node.op); 
           
        
            promote_smaller_reg_size(&left.linkedToRegister, &right.linkedToRegister);

            // Check whether they are signed or not
            int atLeastOneSigned = !left.is_unsigned || !right.is_unsigned;


            switch(binOperation)
            {
                case ADD:
                {
                    fprintf(asm_file, "    add %s, %s\n", left.linkedToRegister->name, right.linkedToRegister->name);
                    break;
                }

                case MUL: 
                {
                    emit_mul(asm_file, left, right);
                    break;
                }


                case SUB: 
                {
                    fprintf(asm_file, "    sub %s , %s\n",left.linkedToRegister->name, right.linkedToRegister->name); 
                    break;
                }


                /* WATCH OUT FOR THIS OPERATION TOO BC HOLY SHIT
                   PROMOTE TO 32 BITS AND FOLLOW THE SAME PATTERN

                   DIVIDEND -> EDX:EAX
                   QUOTIENT -> EAX 
                   REMAINDER -> EDX

                   EDX:EAX / EBX = EAX (QUOTIENT), EDX (REMAINDER)
                */
                case DIV: 
                {
                    emit_div(asm_file, &left, &right);

                    /**
                     * Pin EAX 
                     */
                    pinRegister(EAX);

                    // Return the result in EAX
                    Operand result = { .linkedToRegister = EAX, .is_unsigned = !atLeastOneSigned };
                    //busyRegister(result.linkedToRegister);

                    exit_node(program);
                    return result;
                    break;
                }

                case MOD: 
                {
                    emit_div(asm_file, &left, &right);

                    /**
                     * Pin EDX
                     */
                    pinRegister(EDX);

                    // Return the result in EDX
                    Operand result = { .linkedToRegister = EDX, .is_unsigned = !atLeastOneSigned };
                    //busyRegister(result.linkedToRegister);

                    exit_node(program);
                    return result;
                    break;                   
                }

                case GT: 
                {
                    Register *leftEightBitRegister = getFamilyRegWithSize(left.linkedToRegister,1);
                    fprintf(asm_file, "    cmp %s, %s\n", left.linkedToRegister->name, right.linkedToRegister->name);
                    fprintf(asm_file, "    setg %s\n", leftEightBitRegister->name );
                    left.linkedToRegister = leftEightBitRegister;
                    break;
                }

                case LT: 
                {
                    Register *leftEightBitRegister = getFamilyRegWithSize(left.linkedToRegister,1);
                    fprintf(asm_file, "    cmp %s, %s\n", left.linkedToRegister->name, right.linkedToRegister->name);
                    fprintf(asm_file, "    setl %s\n", leftEightBitRegister->name );
                    left.linkedToRegister = leftEightBitRegister;
                    break;               
                }

                case GTEQ:
                {
                    Register *leftEightBitRegister = getFamilyRegWithSize(left.linkedToRegister,1);
                    fprintf(asm_file, "    cmp %s, %s\n", left.linkedToRegister->name, right.linkedToRegister->name);
                    fprintf(asm_file, "    setge %s\n", leftEightBitRegister->name );
                    left.linkedToRegister = leftEightBitRegister;
                    break;
                }

                case LTEQ:
                {
                    Register *leftEightBitRegister = getFamilyRegWithSize(left.linkedToRegister,1);
                    fprintf(asm_file, "    cmp %s, %s\n", left.linkedToRegister->name, right.linkedToRegister->name);
                    fprintf(asm_file, "    setle %s\n", leftEightBitRegister->name);
                    left.linkedToRegister = leftEightBitRegister;
                    break;
                }

                case EQ:
                {
                    Register *leftEightBitRegister = getFamilyRegWithSize(left.linkedToRegister,1);
                    fprintf(asm_file, "    cmp %s, %s\n", left.linkedToRegister->name, right.linkedToRegister->name);
                    fprintf(asm_file, "    sete %s\n", leftEightBitRegister->name);
                    left.linkedToRegister = leftEightBitRegister;
                    break;
                }

                case NEQ:
                {
                    Register *leftEightBitRegister = getFamilyRegWithSize(left.linkedToRegister,1);
                    fprintf(asm_file, "    cmp %s, %s\n", left.linkedToRegister->name, right.linkedToRegister->name);
                    fprintf(asm_file, "    setne %s\n", leftEightBitRegister->name);
                    left.linkedToRegister = leftEightBitRegister;
                    break;
                }

                case BIT_AND:
                {
                    fprintf(asm_file, "    and %s,%s\n", left.linkedToRegister->name, right.linkedToRegister->name);
                    break;
                }

                case BIT_OR:
                {
                    fprintf(asm_file, "    or %s,%s\n", left.linkedToRegister->name, right.linkedToRegister->name);
                    break;
                }
                
                // logical and 
                case AND:
                {
                    Register *leftEightBitRegister = getFamilyRegWithSize(left.linkedToRegister,1);
                    Register *rightEightBitRegister = getFamilyRegWithSize(right.linkedToRegister, 1);
                    // Test whether the first operand is zero 
                    fprintf(asm_file, "    test %s, %s\n", left.linkedToRegister->name, left.linkedToRegister->name);
                    // Set if non equal 
                    fprintf(asm_file, "    setne %s\n", leftEightBitRegister->name );
                    // Test whether the second operand is zero 
                    fprintf(asm_file, "    test %s, %s\n", right.linkedToRegister->name, right.linkedToRegister->name);
                    // Set if not equal
                    fprintf(asm_file, "    setne %s\n", rightEightBitRegister->name );

                    // Perform and operation 
                    fprintf(asm_file, "    and %s, %s\n", leftEightBitRegister->name, rightEightBitRegister->name);
                    left.linkedToRegister = leftEightBitRegister;


                    break;
                }
                
                // logical or
                case OR: 
                {
                    Register *leftEightBitRegister = getFamilyRegWithSize(left.linkedToRegister,1);
                    Register *rightEightBitRegister = getFamilyRegWithSize(right.linkedToRegister, 1);
                    // Test whether the first operand is zero 
                    fprintf(asm_file, "    test %s, %s\n", left.linkedToRegister->name, left.linkedToRegister->name);
                    // Set if non equal 
                    fprintf(asm_file, "    setne %s\n", leftEightBitRegister->name );
                    // Test whether the second operand is zero 
                    fprintf(asm_file, "    test %s, %s\n", right.linkedToRegister->name, right.linkedToRegister->name);
                    // Set if not equal
                    fprintf(asm_file, "    setne %s\n", rightEightBitRegister->name );

                    // Perform and operation 
                    fprintf(asm_file, "    or %s, %s\n", leftEightBitRegister->name, rightEightBitRegister->name);
                    left.linkedToRegister = leftEightBitRegister;

                    break;                
                }

                default: 
                {
                    fprintf(stderr, "Unsupported binary operation to build. Got: %i \n", binOperation );
                    exit(1);
                }
            
            }
            
            unpinRegister(right.linkedToRegister); 
            
            exit_node(program);

            Operand result = { .linkedToRegister = left.linkedToRegister, .is_unsigned = atLeastOneSigned};
            return result;
        }

        case NODE_STDALONE_POSTFIX_OP:
        {
            enter_node(program);

            PostfixOPCode postfixOPCode = strToPostfixOPCode(program->stdalone_postfix_op_node.op);

            /**
             * Get the address we wanna apply the postfix op at
             */
            Operand operand = with_lvalue_context(asm_file, program->stdalone_postfix_op_node.left, current_st, current_ft);

            /**
             * Request a specific GPR big enough to hold the data we are applying the postfix op to
             */
            Register *temp = request_specific_size_and_type_reg(GPR,program->stdalone_postfix_op_node.size_of_operand);
            Operand op_temp = {.linkedToRegister = temp, .is_unsigned = operand.is_unsigned};

            /**
             * Move into that register the value sitting at the address we wanna aply the postfix at,
             * so the oncoming operations use the value BEFORE applying the postfix op
             */
            emit_mov_a2r(asm_file, &op_temp, &operand, program->stdalone_postfix_op_node.size_of_operand);


            /**
             * Check for whether its a pointer or not. 
             * Different rules apply
             */
            int is_ptr = is_type_ptr(program->stdalone_postfix_op_node.type);


            emit_postfix(asm_file, is_ptr, program->stdalone_postfix_op_node.size_of_operand, operand, postfixOPCode);

            /**
             * Unpin both operand and temp, no longer needed by nobody, this is a stdalone node
             */
            unpinRegister(operand.linkedToRegister);
            unpinRegister(temp);


            // if standalone postfix, nobody needs the register
            Operand result;
            exit_node(program);
            return result;

        }
        case NODE_POSTFIX_OP:
        {
            enter_node(program);


            PostfixOPCode postfixOPCode = strToPostfixOPCode(program->postfix_op_node.op);

            /**
             * Get the address we wanna apply the postfix op at
             */
            Operand operand = with_lvalue_context(asm_file, program->postfix_op_node.left, current_st, current_ft);

            /**
             * Request a specific GPR big enough to hold the data we are applying the postfix op to
             */
            Register *temp = request_specific_size_reg(program->postfix_op_node.size_of_operand);
            Operand op_temp = {.linkedToRegister = temp, .is_unsigned = operand.is_unsigned};
            /**
             * Move into that register the value sitting at the address we wanna aply the postfix at,
             * so the oncoming operations use the value BEFORE applying the postfix op
             */

            emit_mov_a2r(asm_file, &op_temp, &operand, program->postfix_op_node.size_of_operand);

            /**
             * Check for whether its a pointer or not. 
             * Different rules apply
             */
            int is_ptr = is_type_ptr(program->postfix_op_node.type);

            emit_postfix(asm_file, is_ptr, program->postfix_op_node.size_of_operand, operand, postfixOPCode);

            /**
             * Unpin operand, no longer needed what we care about is in temp
             */
             unpinRegister(operand.linkedToRegister);

            exit_node(program);
            return op_temp;
        }

        case NODE_UNARY_OP:
        {
            enter_node(program);

            /**
             * Result is gonna be here
             */
            Operand operand;

            /**
             * Get the unary op code
             */
            UnaryOPCode unaryOPCode = strToUnaryOPCode(program->unary_op_node.op);
            switch(unaryOPCode)
            {
                case UNARY_MINUS:
                {
                    fprintf(asm_file, "    neg %s\n",  operand.linkedToRegister->name);

                    //Not sure of operand.is_unsigned = 0 now
                    operand.is_unsigned = 0;
                    break;
                }

                case UNARY_ADDRESS:
                {
                    emit_comment(asm_file, "Getting the address");

                    operand = with_lvalue_context(asm_file, program->unary_op_node.right, current_st, current_ft);
                    break;
                }

                case UNARY_DEREF:
                {
                    emit_comment(asm_file, "Dereferencing");

                    /**
                     * Get the address, 8 bytes
                     */
                    //operand = with_rvalue_context(asm_file, program->unary_op_node.right, ANY, 8, current_st, current_ft);
                    
                    expectRegister(ANY, 8);
                    operand = buildStart(asm_file, program->unary_op_node.right, current_st, current_ft);
                    clearExpectation();
                    
                    if (getCurrentWorkingContext() != COMPUTING_LVALUE)
                    {
                        // Get the value from the address
                        emit_mov_a2r(asm_file, &operand, &operand, program->unary_op_node.size_of_operand);
                        operand.linkedToRegister = getFamilyRegWithSize(operand.linkedToRegister, program->unary_op_node.size_of_operand);
                    }
                    else 
                    {
                        emit_mov_a2r(asm_file, &operand, &operand, 8);
                    }


                    

                    emit_comment(asm_file, "Finished dereferencing");
                    break;
                }

                default:
                {
                    fprintf(stderr, "Unary op code: %s, not supported yet \n", program->unary_op_node.op);
                    exit(1);
                }
            }

            /**
             * Pin the register holding the value
             */
            pinRegister(operand.linkedToRegister);
            exit_node(program);
            Operand result = { .linkedToRegister = operand.linkedToRegister };
            return result;
        }

        case NODE_IDENTIFIER:
        {

            enter_node(program);

            /**
             * Look up the identifier
             */
            Symbol *identifier = lookup_identifier(current_st, context.current_scope, program->identifier_node.name );
            
            /**
             * Request a register big enough to hold the value
             * TODO: What if there are no expectations? This down will raise error
             */

            Register *reg = request_specific_size_reg(identifier->size);

            /**
             * lea or mov based on whether we are in LVALUE or RVALUE
             */
            char *opcode = getLeaOrMovBasedOnContext();

            // Local variable
            if (identifier->scope >= 1)
            {
                fprintf(asm_file, "    %s %s, [rbp - %i]\n", opcode, reg->name ,getStackBasePtr() - identifier->offset);
            }

            // Global
            else 
            {
                fprintf(asm_file, "    %s %s, [%s] \n", opcode, reg->name, identifier->identifier);
            }

            exit_node(program);

            /**
             * Pin the register, its will be consumed at other point
             */
            pinRegister(reg);

            Operand operand = {.linkedToRegister = reg, .is_unsigned = is_type_unsigned_int(identifier->type)}; 
            return operand;
        }

        case NODE_NUMBER: 
        {
            enter_node(program);

            Operand number;

            /**
             * Get a register big enough to store the number
             */

            Register *reg = request_specific_size_reg(size_of_type(program->number_node.int_type));
            number.linkedToRegister = reg;
            /**
             * Mark it as signed or unsigned (So we choose the right opcodes for binary operations like idiv/div)
             */
            number.is_unsigned = is_type_unsigned_int(program->number_node.int_type);


            emit_mov_imm2r(asm_file, reg, program->number_node.number_value);

            /**
             * Pin the register
             */
            pinRegister(reg);

            exit_node(program);
            return number;
        }

        case NODE_CHAR:
        {
            enter_node(program);
            /**
             * Get a register to hold the char
             */
            Register *reg = request_specific_size_reg(1);
            /**
             * Mark it as unsigned
             */
            Operand character = {.linkedToRegister = reg, .is_unsigned = 1};

            emit_mov_imm2r(asm_file, reg, program->char_node.char_value);

            /**
             * Pin the register
             */

            pinRegister(reg);

            exit_node(program);
            return character;
        }

        case NODE_STR:
        {
            // Load the address of the label 
            Register *reg = request_specific_size_reg(8);
            fprintf(asm_file, "    mov %s, %s\n", reg->name, program->str_node.label);

            Operand str = {.linkedToRegister = reg};
            return str;
        }

        case NODE_BOOL:
        {
            enter_node(program);
            /**
             * Request a register to hold the boolean
             */
            Register *reg = request_specific_size_reg(1);
            /**
             * Mark it as unsigned
             */
            Operand b = {.linkedToRegister = reg, .is_unsigned = 1};

            char boolean = (strcmp(program->bool_node.bool_value, "true") == 0)? 1 : 0;

            emit_mov_imm2r(asm_file, reg, boolean);
            /**
             * Pin the register
             */
            pinRegister(reg);
            exit_node(program);
            
            return b;
            break;
        }

        
        default:
        {
            break;
        }
    }
}
