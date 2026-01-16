


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


SpillReportDrawer SRD = {.report_count = 0, .reports = {0}};

CurrentFrameAndFunction CFF = { .stack_base_ptr = 0, .f = NULL };





/** -------------------------------- UTILS ----------------------------------------- */


int evaluates_to_value(ASTNode *node)
{
    switch(node->node_type)
    {
        case NODE_METHOD_DISPATCH:
        case NODE_PTR_METHOD_DISPATCH:
        case NODE_BINARY_OP:
        case NODE_FIELD_ACCESS:
        case NODE_PTR_FIELD_ACCESS:
        case NODE_SUBSCRIPT:
        case NODE_UNARY_OP: return 1;
        case NODE_FUNC_CALL:
        {
            if (node->is_stmt == 1) { return 0; }
            return 1;
        }

        default: return 0;
    }
}


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

Type **getTypesFromParams(Parameter **params, int param_count)
{
    
    Type **param_types = malloc(sizeof(Type *) * param_count);;

    for (int i = 0; i < param_count; i++){
        param_types[i] = params[i]->type;
    }

    return param_types;
}

Operand *createOperand(int is_unsigned, Register *linkedToRegister)
{
    Operand *operand = malloc(sizeof(Operand));
    operand->is_unsigned = is_unsigned;
    operand->linkedToRegister = linkedToRegister;
}

void SRD_storeReport(SpillReport report)
{
    SRD.reports[SRD.report_count++] = report;
}

void SRD_deleteLastReport()
{
    SRD.report_count--;
}

SpillReport *SRD_getCurrentReport()
{
    if (SRD.report_count == 0)
    {
        fprintf(stderr, "There is NO current SpillReport, can't get it.\n");
        exit(1);
    }

    return &SRD.reports[SRD.report_count - 1];
}

SpillReport SRD_findReport(ASTNode *node)
{
    for (int i = 0; i < SRD.report_count; i++)
    {
        if (SRD.reports[i].ast_responsible == node)
        {
            return SRD.reports[i];
        }
    }

    fprintf(stderr, "Couldn't find SpillReport for node: %s.\n", astTypeToStr(node));
    exit(1);
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

}

void stop_tracking_rsp()
{

    rspTracker.index = max(-1, rspTracker.index - 1);
    if (rspTracker.index == -1)
    {
        rspTracker.is_active = 0;
        return;
    }



    // if (rspTracker.index < -1)
    // {
    //     fprintf(stderr, "After stopping tracking rsp, index = %i.Not allowed index.\n",rspTracker.index);
    //     exit(1);
    // }

    //printf("STOP TRACKING RSP. INDEX IS NOW: %i\n", rspTracker.index);

}

void modify_rsp(int n)
{
    rspTracker.modifications_stack[rspTracker.index].n += n;
}


/*———————————————————————————————————---- REGISTER REQUESTING, SPILLING AND LOGGING —————————————————————————————————————————————————————————  */



void log_spill(Register *reg)
{
    // Get current SpillReport 
    SpillReport *currentReport = SRD_getCurrentReport();

    // Log the register and update the spill count
    currentReport->spilled_regs[currentReport->reg_spilled_count++] = reg;
    
    //printf("Report attached to node: %p,(%s), has now count = %i \n", currentReport->ast_responsible, astTypeToStr(currentReport->ast_responsible) ,currentReport->reg_spilled_count);
}

/**
 * 
 * Spill/Push 'reg' to the stack
 * Unpins it since now it can be used again
 * Logs the spill onto the current SpillReport
 */
void spill_reg(FILE *asm_file, Register *reg)
{
    // Get the parent that will be spilled
    Register *parent = getParent64(reg);

    // Mark it as spilled and the children too
    parent->is_spilled = 1;
    for (int i = 0; i < 3; i++)
    {
        if (parent->subRegisters[i] != NULL)
        {
            parent->subRegisters[i]->is_spilled = 1;
        }
    }

    emit_comment(asm_file, "Spilling to stack");
    // Push it to the stack
    emit_push(asm_file, parent->name);

    // No longer pinned
    unpinRegister(parent);

    // Log the spill
    log_spill(reg);
}

/**
 * Collects/Pops the spilled registers that are NOT pinned until a certain index
 */
void collectSpilledRegisters(FILE *asm_file, SpillReport report, Operand *expr_operand)
{

    if (report.reg_spilled_count == 0) { return; }

    //fprintf(asm_file, "    ; RAX pinned? %i\n", RAX->is_pinned);
    emit_comment(asm_file, "Collecting");

    Register *fresh_reg = NULL;

    // Pop in reverse order
    for (int i = report.reg_spilled_count - 1; i >= 0; i--)
    {
        if (report.spilled_regs[i]->is_pinned)
        {
            Expectation *current_expectation = getCurrentExpectation();
            size_t size_to_move = expr_operand->linkedToRegister->size;

            if (current_expectation != NULL) {
                size_to_move = current_expectation->size;
            }

            //Register *fresh_reg = request_unspilled_specific_size_reg(size_to_move); 
            
            fresh_reg = request_unspilled_ssr(size_to_move);
            unpinRegister(expr_operand->linkedToRegister);
            
            emit_mov_r2r(asm_file, fresh_reg, expr_operand->linkedToRegister, size_to_move, expr_operand->is_unsigned);
            pinRegister(fresh_reg);
            // fprintf(asm_file, "    ; Just pinned %s\n", fresh_reg->name);

            // Modify the expr_operand, has to be linked to the new fresh reg
            expr_operand->linkedToRegister = fresh_reg;
        }

        emit_pop(asm_file, report.spilled_regs[i]->name);
        pinRegister(report.spilled_regs[i]);
        // fprintf(asm_file, "    ;Just pinned by popping: %s\n", report.spilled_regs[i]->name);
        report.spilled_regs[i]->is_spilled = 0;
    }

    // We can unpin registers that we didnt pop and also not the fresh one ofc
    for (int i = 0; i < gpr->num_of_registers; i++)
    {
        if (!is_in_spill_report(gpr->registers[i], &report))
        {
            // fprintf(asm_file, "    ;About to unpin %s\n", gpr->registers[i]->name);
            unpinRegister(gpr->registers[i]);
        }
    }

    if (fresh_reg != NULL) {  pinRegister(fresh_reg); }
    report.reg_spilled_count = 0;

}


/**
 *  Pins a register and also the entire family
 */
void pinRegister(Register *reg)
{
    if (reg == NULL)
    {
        fprintf(stderr, "Can't pin a NULL register\n");
        exit(1);
    }
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
 * 
 */
void unpinRegister(Register *reg)
{

    Register *parent = getParent64(reg);

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
 * Loads an immediate value onto a register. DOES NOT pin it
 * If there is an expectation, the register size and type will match the expected size and type
 * Else, uses a GPR register and the size of the immediate  
 * RETURNS: An Operand linked to the register and sign specification
 */
Operand *load_imm(FILE *asm_file, size_t size, ASTNode *imm_node)
{
    Expectation *curr_exp = getCurrentExpectation();

    Register *reg;
    Operand *result = createOperand(0, NULL);

    /**
     * If the expectation is NULL, get a register fit for the immediate size
     */
    if (curr_exp == NULL)
    {
        reg = getGPR(asm_file, size);
    }

    /**
     * Else, get a register matching the expected size
     */
    else 
    {
        switch (curr_exp->registerType)
        {
            case ANY:
            case GPR: reg = getGPR(asm_file, curr_exp->size); break;
            case FPR: reg = getFPR(curr_exp->size); break;
        }
    }

    /**
     * Emit the right instructions based on immediate node types
     */
    switch(imm_node->node_type)
    {
        case NODE_SIZEOF:{
            emit_mov_imm2r(asm_file, reg, calculateSizeOfType(imm_node->sizeof_node.type));
            result->is_unsigned = isTypeUnsignedInt(imm_node->sizeof_node.type);
            break;
        }
        case NODE_NUMBER:
        {
            emit_mov_imm2r(asm_file, reg, imm_node->number_node.number_value);
            result->is_unsigned = isTypeUnsignedInt(imm_node->number_node.number_type);
            break;
        }
        case NODE_CHAR:
        {
            emit_mov_imm2r(asm_file, reg, imm_node->char_node.char_value);
            result->is_unsigned = 1;
            break;
        }
        case NODE_BOOL:
        {
            uint8_t bool_value = (strcmp(imm_node->bool_node.bool_value, "true") == 0)? 1 : 0;
            emit_mov_imm2r(asm_file, reg, bool_value);
            result->is_unsigned = 1;
            break;
        }
        case NODE_STR:
        {
            emit_mov_label2r(asm_file, reg, imm_node->str_node.label->tag.tag_name);
            // I guess a label its unsigned?
            break;
        }
    }
    /**
     * Return the operand result
     */     

    result->linkedToRegister = reg;     
    return result;
}

/**
 * Loads an identifier value onto a register. DOES NOT pin it
 * DOES NOT care about expectations, identifiers need their whole size read
 * If calculating LVALUE, gets an 8 byte register in GPR
 * Else, gets a GPR register with size matching the identifier size
 * RETURNS: An Operand linked to the register and sign specification
 */
Operand *load_identifier(FILE *asm_file, Symbol *s)
{
    Register *reg;
    Operand *result = createOperand(isTypeUnsignedInt(s->smi->type), NULL);

    /**
     * Get the correct register. 8 bytes if in LVALUE (address),
     * whatever bytes if in RVALUE (actual value)
     */
    if (getCurrentWorkingContext() == COMPUTING_LVALUE){
        reg = getGPR(asm_file, 8);
    }
    else {
        reg = getGPR(asm_file, s->smi->size);
    }

    /**
     * Correct move operation based on whether the identifier is global or local
     */
    if (s->smi->scope == 0){
        emit_mov_derefLabel2r(asm_file, reg, s->smi->identifier);
    }
    else {
        emit_mov_s2r(asm_file, reg, s->smi->offset, s->smi->size, result->is_unsigned);
    }

    /**
     * Return the result
     */
    result->linkedToRegister = reg;
    return result;
}

/**
 * Tries to get the SPECIFIC wanted register
 * If it is pinned, spills to the stack and returns it
 * It DOES NOT pin the reegister
 * RETURNS: The SPECIFIC wanted register
 */
Register *request_specific_reg(FILE *asm_file, Register *reg)
{
    
    // If it's not in use, get it
    if (!reg->is_pinned) { return reg; }

    // Ensure we have the parent 
    Register *parent = getParent64(reg);

    spill_reg(asm_file, parent);

    // Return the register requested
    return reg;
}


/**
 * Tries to get a register that is not pinned and has not been spilled, that is
 * a complete free register
 * It DOES NOT pin the returned register.
 * RETURNS: an UNPINNED and UNSPILLED GPR register
 */
Register *request_unspilled_specific_size_reg(int size)
{   
    return getUnspilledGPR(size);
}

int is_in_spill_report(Register *reg, SpillReport *report)
{
    for (int i = 0; i < report->reg_spilled_count; i++)
    {
        if (are_reg_family(reg, report->spilled_regs[i])) { return 1; }
    }
    return 0;
}

Register *request_unspilled_ssr(int size)
{
    SpillReport *currentReport = SRD_getCurrentReport();
    Register **spilled_regs = currentReport->spilled_regs;

    for (int i = 0; i < gpr->num_of_registers; i++)
    {
        if (is_in_spill_report(gpr->registers[i], currentReport) || gpr->registers[i]->is_pinned) { continue; } 
        switch (size)
        {
            case 8: { return gpr->registers[i]; }
            case 4: { return gpr->registers[i]->subRegisters[0]; }
            case 2: { return gpr->registers[i]->subRegisters[1]; }
            case 1: { return gpr->registers[i]->subRegisters[2]; }
            default:
            {
                fprintf(stderr, "Cannot get a GPR register for size: %i\n", size);
                exit(1);
            }
        }
    }

    fprintf(stderr, "I DONT KNOW MAN\n");
    exit(1);

}

/**
 * It tries to get an unpinned register matching the provided size, if they are all pinned, it spills to stack, unpins that one and returns it
 * It DOES NOT pin the returned register.
 * RETURNS: An unpinned GPR register matching the provided size.
 */
Register *request_specific_size_reg(FILE *asm_file, int size)
{
    return getGPR(asm_file, size);
}

/**
 * 
 * It tries to get an unpinned register matching the provided size and type.
 * If rType = GPR and they are all pinned, spills to stack, unpins that one and returns it
 * It DOES NOT pin the returned register.
 */
Register *request_specific_size_and_type_reg(FILE *asm_file, RegisterType rType, int size)
{

    switch (rType)
    {
        case ANY:
        case GPR: return getGPR(asm_file, size);
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
 * 
 * 8/12: FIX to look like promote_bigger_reg_size()
 */
// void promote_smaller_reg_size(Register **regA, Register **regB)
// {
//     if ((*regA)->size != (*regB)->size)
//     {
//         int min_size = min((*regA)->size, (*regB)->size);

//         *regA = getFamilyRegWithSize(*regA, min_size);
//         *regB = getFamilyRegWithSize(*regB, min_size);
//     }
// }


/**
 * 
 * From the two passed registers, the bigger size is promoted, that is,
 * the register with the bigger size is swapped for one of the same family with the bigger size
 */
void promote_bigger_reg_size(FILE *asm_file, Register **left, Register **right, int is_unsigned_right, int is_unsigned_left)
{
    if ((*left)->size == (*right)->size) { return; }

    // If left is bigger, we need to extend right
    if ((*left)->size > (*right)->size)
    {

        emit_mov_r2r(asm_file, getFamilyRegWithSize(*right, (*left)->size), *right, (*left)->size, is_unsigned_right);
        *right = getFamilyRegWithSize(*right,(*left)->size);
        fprintf(asm_file, "    ;Finished promoting bigger size\n");

        return;
    }

    // If regB is bigger, we need to extend regA
    emit_mov_r2r(asm_file, getFamilyRegWithSize(*left, (*right)->size), *left, (*right)->size, is_unsigned_left);
    *left = getFamilyRegWithSize(*left,(*right)->size);
    fprintf(asm_file, "    ;Finished promoting bigger size\n");

}



/*———————————————————————————————————---- END REGISTER REQUESTING, SPILLING AND LOGGING —————————————————————————————————————————————————————————  */





/** -------------------------------- END UTILS ----------------------------------------- */









/** -------------------------------- EMIT ----------------------------------------- */

// Writes the comment to the asm file with \n at the end
void emit_comment(FILE *asm_file, char *comment)
{
    fprintf(asm_file, "    ;%s\n", comment);
}

/**
 * Emits an operation mov regX, imm
 */
void emit_mov_imm2r(FILE *asm_file, Register *dst, uint64_t n)
{
    fprintf(asm_file, "    mov %s, %"PRId64"\n", dst->name, n);
}

/**
 * Emits an operation mov/lea regX, label
 */
void emit_mov_label2r(FILE *asm_file, Register *dst, char *label)
{
    fprintf(asm_file, "    %s %s, %s\n", getLeaOrMovBasedOnContext(),dst->name, label);
}

/**
 * Emits an operation mov/lea regX, [label]
 */
void emit_mov_derefLabel2r(FILE *asm_file, Register *dst, char *label)
{
    fprintf(asm_file, "    %s %s, [%s]\n", getLeaOrMovBasedOnContext() ,dst->name, label);
}

/**
 * Emits an operation mov (movzx/movsx) regA, regB ensuring regA is of size 'size_to_preserve'
 * Moves the value in src to dst
 */
void emit_mov_r2r(FILE *asm_file, Register *dst, Register *src, int bytes_to_move, int is_unsigned)
{   
    if (dst == src) { return; }

    //printf("GOT HERE \n");
    // Get the destination register able to hold the size to preserve
    // Nothing to do here
    if ( (src->size == dst->size) && are_reg_family(dst,src)) { return; }

    //printf("SIZED DST = %s \n", sized_dst->name);
    // If they have the same size, just move it
    if (src->size == dst->size){
        fprintf(asm_file, "    mov %s, %s\n", dst->name, src->name);

        return;
    }

    // Special case for mov r8, r4
    if ( dst->size == 8 && src->size == 4) {
        if (are_reg_family(dst, src)){
            return;
        }
        else {
            // mov eax, ebx basically --> automatically zero extends upper bits BUT WE COULD LOSE INFO BC MAYBE THERE'S STUFF IN RBX
            // mov rax, rbx could be better
            if (is_unsigned){
                fprintf(asm_file, "    mov %s, %s\n", dst->name, src->parent64Register->name);
                return;
            }
            // if it's signed, gotta do movsxd rax, ebx 
            fprintf(asm_file, "    movsxd %s, %s \n", dst->name, src->name);
            return;
        }
    }


    // At this point, we have two registers with different sizes, we have to extend src onto dst
    // Instruction is different whether we want to preserve signed or unsigned bits
    Register *sized_dst = getFamilyRegWithSize(dst, bytes_to_move);

    // Large to small, truncate to smaller size
    if (src->size > dst->size)
    {
        fprintf(asm_file, "    mov %s, %s\n", sized_dst->name, getFamilyRegWithSize(src,bytes_to_move)->name);
        return;    
    }

    // Small to large
    char *movOperation = (is_unsigned)? "movzx" : "movsx";

    fprintf(asm_file, "    %s %s, %s\n", movOperation ,dst->name, src->name);

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
 * Emits an operation mov/lea regA, [rbp - offset]
 * Moves the value in the stack to regA
 */
void emit_mov_s2r(FILE *asm_file, Register *dst, int offset, int bytes_to_read, int is_unsigned)
{



    if (getCurrentWorkingContext() == COMPUTING_LVALUE)
    {
        if (dst->size != 8)
        {
            fprintf(stderr, "Trying to do lea onto a register that's not 8 bytes long: %s\n", dst->name);
            exit(1);
        }
        fprintf(asm_file, "    lea %s, [rbp - %i]\n", dst->name, getStackBasePtr() - offset);
        return;
    }

    //If the destination size matches the amount of bytes we want to read from the value stored in the stack, we good
    if (dst->size == bytes_to_read)
    {
        fprintf(asm_file, "    mov %s, [rbp - %i]\n", dst->name, getStackBasePtr() - offset);
        return;
    }

    // At this point the destination size doesnt match the amount of bytes to read, we must extend
    char *movOperation = (is_unsigned)? "movzx" : "movsx";
    char *word = getWordForSize(bytes_to_read);

    fprintf(asm_file, "    %s %s, %s [rbp - %i]\n", movOperation, dst->name, word, getStackBasePtr() - offset);


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
        fprintf(asm_file, "    mov [rbp - %i], %s\n", getStackBasePtr() - stack_offset, op_src->linkedToRegister->name);
        return;
    }

    // At this point, the source register size does not match the amount of bytes we want to move
    // Get the family register that does

    Register *sized_reg = getFamilyRegWithSize(op_src->linkedToRegister, bytes_to_move);

    fprintf(asm_file, "    mov [rbp - %i], %s\n", getStackBasePtr() - stack_offset, sized_reg->name);
}


// 24/11 - Change this? 
void emit_mul(FILE *asm_file, Operand *left, Operand *right )
{
    // If expected size is 8 bits, for example: int8 x = 3 * 4, we could end up having
    // imul al,bl which is not valid x86 syntax

    // Result in AX
    if (left->linkedToRegister->size == 1 && right->linkedToRegister->size == 1)
    {
        int left_in_rax = are_reg_family(left->linkedToRegister, RAX);
        int right_in_rax = are_reg_family(right->linkedToRegister, RAX);

        // If none is in RAX, move one of them there, for example left, and multiply by the other one, right
        if (!(left_in_rax || right_in_rax)){
            Register *rax = request_specific_reg(asm_file, RAX);
            emit_mov_r2r(asm_file, rax, left->linkedToRegister, 1, left->is_unsigned);
            fprintf(asm_file, "    imul %s \n", right->linkedToRegister->name);
            unpinRegister(right->linkedToRegister);
        }
        // If at least one is in RAX, multiply by the other one
        else {
            Register *multiplier = (left_in_rax)? right->linkedToRegister : left->linkedToRegister;
            fprintf(asm_file, "    imul %s \n", multiplier->name);

            if (left_in_rax){
                unpinRegister(right->linkedToRegister);
            }
            else {
                unpinRegister(left->linkedToRegister);
            }
        }

        return;
    }

    // This can be whatever
    // Result in Left
    else 
    {
        fprintf(asm_file, "    imul %s, %s\n", left->linkedToRegister->name, right->linkedToRegister->name);
    }

    unpinRegister(right->linkedToRegister);
}

// Performs multiply with imul r,r,imm
// Wonderful
void emit_mul_imm(FILE *asm_file, Operand *left, int number)
{
    fprintf(asm_file, "    imul %s, %s, %i \n", left->linkedToRegister->name, left->linkedToRegister->name, number);
}

void emit_add(FILE *asm_file, Register *left, Register *right)
{
    fprintf(asm_file, "    add %s, %s\n", left->name, right->name);
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

    int atLeastOneSigned = !(left->is_unsigned && right->is_unsigned);
    //fprintf(asm_file, "    ;At least one signed? %i\n", atLeastOneSigned);
    char *divCode = atLeastOneSigned ? "idiv" : "div";
    
    // Determine size (use the larger of the two, or at least 4 for division)
    int div_size = (left->linkedToRegister->size > right->linkedToRegister->size) 
                   ? left->linkedToRegister->size : right->linkedToRegister->size;
    if (div_size < 4) div_size = 4;  // Division needs at least 32-bit


    Register *RAX_divSized = getFamilyRegWithSize(RAX, div_size);
    Register *RBX_divSized = getFamilyRegWithSize(RBX, div_size);

    // Load dividend to RAX/EAX  
    if (left->linkedToRegister != RAX_divSized && !are_reg_family(left->linkedToRegister, RAX))  
    {
        Register *eax = request_specific_reg(asm_file, RAX_divSized);
    }
    
    // Use left operand's actual signedness
    emit_mov_r2r(asm_file, RAX_divSized, left->linkedToRegister, div_size, left->is_unsigned);
    unpinRegister(left->linkedToRegister);



    // Load divisor to EBX
    if (right->linkedToRegister != RBX_divSized && !are_reg_family(right->linkedToRegister, RBX))
    {
        Register *ebx = request_specific_reg(asm_file, RBX_divSized);
    }

    emit_mov_r2r(asm_file, RBX_divSized, right->linkedToRegister, div_size, right->is_unsigned);

    // Prepare EDX
    if (atLeastOneSigned)
    {
        Register *rdx = request_specific_reg(asm_file, RDX);
        if (div_size == 4) {
            fprintf(asm_file, "    cdq\n");     // Sign extend EAX to EDX:EAX (32-bit)
        } else {
            fprintf(asm_file, "    cqo\n");     // Sign extend RAX to RDX:RAX (64-bit)
        }
    }
    else 
    {
        if (div_size == 4) {
            fprintf(asm_file, "    xor edx, edx\n");  // Clear EDX for 32-bit
        } else {
            fprintf(asm_file, "    xor rdx, rdx\n");  // Clear RDX for 64-bit
        }
    }
    //fprintf(asm_file, "Just before moving to ebx, right unsigned? %i\n", right->is_unsigned);
    // Use right operand's actual signedness, not atLeastOneSigned!
    unpinRegister(right->linkedToRegister);
    fprintf(asm_file, "    %s %s\n", divCode, (div_size == 4) ? "ebx" : "rbx");

}




void emit_extern_function(FILE *asm_file, char *func_signature)
{
    fprintf(asm_file, "extern %s\n", func_signature);
}

void emit_label(FILE *asm_file, char *label)
{
    if (label == NULL){
        fprintf(stderr, "Cant emit a NULL label name\n");
        exit(1);
    }
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
    lvalue = with_lvalue_context(asm_file, reassignment_node->lvalue, current_st, current_ft);

    /**
     * Compute the RValue address we want to read from
     */
    emit_comment(asm_file, "RValue (address)");
    rvalue = with_lvalue_context(asm_file, reassignment_node->expression, current_st, current_ft);

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
void emit_ptr_reassign(FILE *asm_file, Register *left, Register *right, Type *pointed_type, TokenType REASSIGN_TOKEN)
{
    //Register *sized_right = getFamilyRegWithSize(right, pointed_size);

    switch(REASSIGN_TOKEN)
    {
        case TOKEN_ASSIGN:
        {
            fprintf(asm_file, "    mov [%s], %s \n", left->name, right->name);
            break;
        }

        case TOKEN_ADD_ASSIGN:
        {
            int pointed_size = 1;
            if (!isTypeOfKind(pointed_type, TYPE_VOID)){
                pointed_size = calculateSizeOfType(pointed_type);
            }
            fprintf(asm_file, "    imul %s, %i \n", right->name, pointed_size);
            fprintf(asm_file, "    add [%s], %s \n", left->name, right->name);
            break;
        }

        case TOKEN_SUB_ASSIGN:
        {
            int pointed_size = 1;
            if (!isTypeOfKind(pointed_type, TYPE_VOID)){
                pointed_size = calculateSizeOfType(pointed_type);
            }
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
void emit_scalar_reassign(FILE *asm_file, Operand *left, Operand *right, int reassignment_size, TokenType REASSIGN_TOKEN)
{
    switch(REASSIGN_TOKEN)
    {
        case TOKEN_ASSIGN:
        {
            fprintf(asm_file, "    mov %s [%s], %s \n", getWordForSize(reassignment_size), left->linkedToRegister->name, right->linkedToRegister->name);
            break;
        }

        case TOKEN_ADD_ASSIGN:
        {
            promote_bigger_reg_size(asm_file, &left->linkedToRegister, &right->linkedToRegister, right->is_unsigned, left->is_unsigned);
            fprintf(asm_file, "    add [%s], %s \n", left->linkedToRegister->name, right->linkedToRegister->name);
            break;
        }

        case TOKEN_SUB_ASSIGN:
        {
            promote_bigger_reg_size(asm_file, &left->linkedToRegister, &right->linkedToRegister, right->is_unsigned, left->is_unsigned);
            fprintf(asm_file, "    sub [%s], %s \n", left->linkedToRegister->name, right->linkedToRegister->name);
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
    * Compute rvalue and lvalue
    * If we compute rvalue first, we avoid certain issues where rvalue and lvalue may be put onto the same register (RAX)
    */
   /**
    * But for now, compute lvalue first so we now it works
    */
    lvalue = with_lvalue_context(asm_file, reassignment_node->lvalue, current_st, current_ft);
    rvalue = with_rvalue_context(asm_file, reassignment_node->expression, GPR, reassignment_node->size, current_st, current_ft);


    // Pointer reassignment
    if (isTypeOfKind(reassignment_node->type, TYPE_PTR))
    { 
        emit_ptr_reassign(asm_file, lvalue->linkedToRegister, rvalue->linkedToRegister, reassignment_node->type->pointerType.pointed_type, reassignment_node->op);
        return;  
    } 

    emit_comment(asm_file, "Reassign move instruction");
    fprintf(asm_file, "    ;With size = %i\n", reassignment_node->size);
    // Scalar reassignment
    emit_scalar_reassign(asm_file, lvalue, rvalue, reassignment_node->size, reassignment_node->op);

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



void emit_binop_deepest(FILE *asm_file, BinaryOPNode *binary_op_node, Operand **left, Operand **right, SymbolTable *current_st, FunctionTable *current_ft)
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
    

    pinRegister((*right)->linkedToRegister);
    pinRegister((*left)->linkedToRegister);

}


//**  -------------------------------------- END EMIT --------------------------------------------- **//

void updateCFF(Function *f)
{
    CFF.f = f;
    // RSP has to be subtracted enough to account for the weight of all variables AND the padding in between them
    //printf("FUNCTION: %s, has weight: %i, and padding: %i \n", f->identifier, f->local_symbols->weight, f->local_symbols->padding);
    CFF.stack_base_ptr = align_rsp(f->fmi->local_symbols->weight + f->fmi->local_symbols->padding);

    //printf("UPDATED CFF. Stack Base Pointer = %i\n", CFF.stack_base_ptr);

    //printf("Total weight and padding for function %s : %i + %i = %i\n", f->identifier,f->local_symbols->weight, f->local_symbols->padding,
    //        f->local_symbols->weight + f->local_symbols->padding);
}

Function *getCurrentFunction() { return CFF.f; }

int getStackBasePtr() { return CFF.stack_base_ptr; }


void enter_node(FILE *asm_file, ASTNode *node)
{
    //fprintf(asm_file, "    ;Entering node %s\n", astTypeToStr(node));

    if (is_pushing_scope(node))
    {
        pushContext(astToContext(node));
    }

    

    // 2. Clear the registers, we may be pushing a scope within another one
    if (is_statement(node))
    {
        clearRegisters(gpr);
        clearRegisters(fpr);
        clearAllExpectations();
        popAllWorkingContexts();
    }

    // 3. If it evaluates to a value, create a spill report 
    // Push pinned register
    if (evaluates_to_value(node))
    {
        SpillReport report = {.ast_responsible = node, .reg_spilled_count = 0, .spilled_regs = {0}};
        SRD_storeReport(report);

        // If it's a func call, push all pinned registers
        if (node->node_type == NODE_FUNC_CALL)
        {
            // Start tracking rsp
            start_tracking_rsp();
            int count = 0;
            Register **pinneds = getPinnedGPRs(&count);
            for (int i = 0; i < count; i++)
            {
                spill_reg(asm_file, pinneds[i]);
            }
        }
    }
}

// 1.Pop bt_context if scope has been pushed 
// 2.Clear all registers if the node is a complete statement 
// 3.Clear all expectations, there is nothing to expect anymore 
// 4.Pop all working contexts, we aren't working in anything 
void exit_node(FILE *asm_file, ASTNode *node, Operand *just_calculated)
{
    //fprintf(asm_file, "    ;Exiting node %s\n", astTypeToStr(node));

    // printf("EXITING NODE %s, %p\n", astTypeToStr(node), node);
    if (is_pushing_scope(node))
    {
        popContext();
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
    
    // If it evaluates to a value, clean up
    if (evaluates_to_value(node))
    {
        //fprintf(asm_file, "    ;Exiting node %s\n", astTypeToStr(node));

        // Store the just calculated operand
        node->operand = just_calculated;

        // Find the spill report 
        SpillReport report = SRD_findReport(node);
        // printf("Found the report attached to node: %p, with count = %i\n", node, report.reg_spilled_count);

        // Collect spilled registers
        collectSpilledRegisters(asm_file, report, node->operand);

        // Delete the last report (this one)
        SRD_deleteLastReport();
    }
    

}



Operand *with_lvalue_context(FILE *asm_file, ASTNode *node, SymbolTable *st, FunctionTable *ft)
{
    pushWorkingContext(COMPUTING_LVALUE);
    expectRegister(ANY, 8);
    Operand *result = buildStart(asm_file, node, st, ft);
    clearExpectation();
    popWorkingContext(); 
    return result;
}

Operand *with_rvalue_context(FILE *asm_file, ASTNode *node, RegisterType rtype, int size, SymbolTable *st, FunctionTable *ft)
{

    pushWorkingContext(COMPUTING_RVALUE);
    expectRegister(rtype, size);

    Operand *result = buildStart(asm_file, node, st, ft);

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
        case NODE_ELSEIF: return CTX_ELSEIF;
        case NODE_WHILE: return CTX_WHILE;
        case NODE_FOR: return CTX_FOR;
        case NODE_FOREACH: return CTX_FOREACH;
        case NODE_CLASS: return CTX_CLASS;
        case NODE_OBJECT: return CTX_OBJECT;
        case NODE_ENUM: return CTX_ENUM;
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
 * Gets an UNPINNED (SPILLED OR NOT) GPR register for the specified size. 
 * 
 */
Register *getGPR(FILE *asm_file, int size)
{
    /**
     * First, try to get a unpinned register
     */
    for (int i = 0; i < gpr->num_of_registers; i++)
    {
        if (gpr->registers[i]->is_pinned == 1) { continue; } 
        switch (size)
        {
            case 8: { return gpr->registers[i]; }
            case 4: { return gpr->registers[i]->subRegisters[0]; }
            case 2: { return gpr->registers[i]->subRegisters[1]; }
            case 1: { return gpr->registers[i]->subRegisters[2]; }
            default:
            {
                fprintf(stderr, "Cannot get a GPR register for size: %i\n", size);
                exit(1);
            }
        }
    }
    /**
     * If all of the registers are pinned we end up here
     * Spill one and re-use it
     */
    for (int i = 0; i < gpr->num_of_registers; i++)
    {
        switch(size)
        {
            case 8: { spill_reg(asm_file, gpr->registers[i]); return gpr->registers[i]; }
            case 4: { spill_reg(asm_file, gpr->registers[i]); return gpr->registers[i]->subRegisters[0]; }
            case 2: { spill_reg(asm_file, gpr->registers[i]); return gpr->registers[i]->subRegisters[1]; }
            case 1: { spill_reg(asm_file, gpr->registers[i]); return gpr->registers[i]->subRegisters[2]; }
            default:
            {
                fprintf(stderr, "Cannot get a GPR register for size: %i\n", size);
                exit(1);
            }
        }
    }
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

    fprintf(stderr, "Can't get unspilled GPR register for size: %i. All are being used\n", size);
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
        case NODE_SIZEOF:
        case NODE_IDENTIFIER: return depth;
        

        case NODE_POSTFIX_OP:
        case NODE_UNARY_OP: return compute_binaryOperationDepth(node->unary_op_node.right, depth + 1);



        case NODE_BINARY_OP:
        {
            int sub_left = compute_binaryOperationDepth(node->binary_op_node.left, depth + 1);
            int sub_right = compute_binaryOperationDepth(node->binary_op_node.right, depth + 1);
            return sub_left > sub_right ? sub_left : sub_right;
        }

        case NODE_METHOD_DISPATCH:
        case NODE_PTR_METHOD_DISPATCH:
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


char *get_if_signature()
{
    char *label = malloc(100 * sizeof(char));
    
    sprintf(label, "if_stmt_%i", context.if_block_id);

    return label;
}

char *get_elseif_signature()
{
    char *label = malloc(100 * sizeof(char));
    
    sprintf(label, "elseif_stmt_%i", context.elseif_block_id);

    return label;
}

char *get_else_signature()
{
    char *label = malloc(100 * sizeof(char));
    
    sprintf(label, "else_stmt_%i", context.else_block_id);

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

        case NODE_CLASS:
        {
            for (int i = 0; i < program->class_node.funcdefs_count; i++)
            {
                collectStrings(asm_file, program->class_node.func_defs[i]);
            }
            break;
        }

        case NODE_FUNC_DEF:
        {
            collectStrings(asm_file, program->funcdef_node.body);
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
            collectStrings(asm_file, program->for_node.assignment_expr);
            collectStrings(asm_file, program->for_node.condition_expr);
            collectStrings(asm_file, program->for_node.reassignment_expr);
            collectStrings(asm_file, program->for_node.body);
            break;
        }

        case NODE_WHILE:
        {
            collectStrings(asm_file, program->while_node.condition_expr);
            collectStrings(asm_file, program->while_node.body);
            break;
        }

        case NODE_FOREACH:
        {
            collectStrings(asm_file, program->foreach_node.foreach_lowered);
            break;
        }

        case NODE_IF:
        {
            collectStrings(asm_file, program->if_node.condition_expr);
            collectStrings(asm_file, program->if_node.body);


            for (int i = 0; i < program->if_node.elseif_count; i++)
            {
                collectStrings(asm_file, program->if_node.elseif_nodes[i]);
            }

            if (program->if_node.else_body != NULL)
            {
                collectStrings(asm_file, program->if_node.else_body);
            }
            break;
        }

        case NODE_ELSEIF:
        {
            collectStrings(asm_file, program->elseif_node.condition_expr);
            collectStrings(asm_file, program->elseif_node.body);
            break;

        }

        case NODE_ARRAY_INIT:
        {
            for (int i = 0; i < program->array_init_node.element_count; i++)
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
            fprintf(asm_file, "%s: db ", program->str_node.label->tag.tag_name);

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
    //printf("Building data for node: %s \n", astTypeToStr(program));
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

        case NODE_UNARY_OP:
        {
            buildData(asm_file, program->unary_op_node.right, current_st, current_ft);
            break;
        }

        case NODE_BINARY_OP:
        {
            buildData(asm_file, program->binary_op_node.left, current_st, current_ft);
            buildData(asm_file, program->binary_op_node.right, current_st, current_ft);
            break;
        }

        case NODE_ENUM:
        {
            int count = program->enum_node.declaration_count;

            for (int i = 0; i < count; i++)
            {
                fprintf(asm_file, "%s db %i\n", program->enum_node.declarations[i]->declaration_node.identifier, i);
            }

            break;
        }

        case NODE_FUNC_DEF:
        {


            // For proper function resolution under right scope
            pushContext(CTX_FUNCTION);


            Type **param_types = getTypesFromParams(program->funcdef_node.params, program->funcdef_node.params_count);;


            //printf("HERE \n");
            // Function *f = lookUpFunction(current_ft, context.current_scope, 
            //                                 program->funcdef_node.func_name, param_types, program->funcdef_node.params_count);
            Function *f = program->funcdef_node.function;

            //printf("WE FOUND f: %s, getting inside\n", f->identifier);
            buildData(asm_file, program->funcdef_node.body, f->fmi->local_symbols, f->fmi->local_functions);
            //printf("GOT OUT\n");

            popContext();

            break;
        }

        
        case NODE_ASSIGNMENT:
        {
            int size = calculateSizeOfType(program->assignment_node.type);
            

            // Try find the identifier in the global scope
            Symbol *s = lookUpSymbol(current_st, 0, program->assignment_node.identifier);


            if (s == NULL) { return; }

            char *define_enough_bytes = defineWordForSize(calculateSizeOfType(program->assignment_node.type));

            fprintf(asm_file, "%s %s ", s->smi->identifier, define_enough_bytes);
            buildData(asm_file, program->assignment_node.expression, current_st, current_ft);            

            break;
        }


        // 27/11 - What happens if we do global str s; ?? dunno 
        // gotta look into that
        case NODE_DECLARATION:
        {
            int size = calculateSizeOfType(program->declaration_node.type);
             

            Symbol *s = lookUpSymbol(current_st, 0, program->declaration_node.identifier);

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
            Type **param_types = getTypesFromParams(program->extern_func_def_node.params, program->extern_func_def_node.params_count);;
            //Function *extern_function = lookUpFunction(gb_functionTable,0,program->extern_func_def_node.func_name,param_types, program->extern_func_def_node.params_count);
            Function *extern_function = program->extern_func_def_node.function;

            emit_extern_function(asm_file, extern_function->fmi->reference_labels->compound_label.start_tag->tag_name);
            Operand none;
            return none;
        }

        default:
            break;


    }
}

Operand * buildStart(FILE *asm_file, ASTNode *program, SymbolTable *current_st, FunctionTable *current_ft)
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
            enter_node(asm_file, program);
            for (int i = 0; i < program->block_node.statement_count; i++)
            {
               buildStart(asm_file, program->block_node.statements[i], current_st, current_ft); 
            }
            
            exit_node(asm_file, program, NULL);
            //printf("FINISHED BLOCK\n");
            return NULL;
        }

        case NODE_USE_DIRECTIVE:
        {
            enter_node(asm_file,program);

            buildStart(asm_file, program->use_node.program, current_st, current_ft);

            exit_node(asm_file,program, NULL);

            return NULL;
        }



        case NODE_CAST:
        {
            enter_node(asm_file,program);
            Operand *casting_expr = buildStart(asm_file, program->cast_node.expr, current_st, current_ft); 
            exit_node(asm_file,program, casting_expr);
            
            return casting_expr;
        }
        case NODE_SIZEOF:{
            enter_node(asm_file, program);
            Operand *size = load_imm(asm_file, 8, program);    // Sizeof() result is going to be on a 8 byte register
            pinRegister(size->linkedToRegister);
            exit_node(asm_file, program, size);
            return size;
        }

        case NODE_WHILE:
        {
            enter_node(asm_file,program);
            if (program->while_node.label == NULL){
                printf("ASDASD\n");
            }
            Tag *start_tag = program->while_node.label->compound_label.start_tag;
            Tag *end_tag = program->while_node.label->compound_label.end_tag;

            emit_label(asm_file, start_tag->tag_name);
            // Condition check
            emit_comment(asm_file, "While loop condition check");

            Operand *condition = buildStart(asm_file, program->while_node.condition_expr, current_st, current_ft);
            
            // Compare with 0. Use the value sitting in condition.linkedToRegister
            fprintf(asm_file, "    cmp %s, 0\n", condition->linkedToRegister->name);

            // We can unpin now, the condition has already been checked
            // Clear the registers, nothing is longer needed
            unpinRegister(condition->linkedToRegister);
            clearRegisters(gpr);

            // Jump to end while label if condition false
            fprintf(asm_file, "    je %s\n",end_tag->tag_name);

            // While body
            emit_comment(asm_file, "Body");
            buildStart(asm_file, program->while_node.body, current_st, current_ft);

            // Back to while loop condition check
            emit_comment(asm_file, "Back to while loop condition check");
            fprintf(asm_file, "    jmp %s\n\n", start_tag->tag_name);

            // End while loop label
            emit_label(asm_file, end_tag->tag_name);
            
            exit_node(asm_file,program, NULL);
            //printf("FINISHED WHILE\n");

            break;
        }

       
        case NODE_FOREACH:
        {
            enter_node(asm_file,program);

            emit_comment(asm_file, "Lowered foreach node");
            printf("---------- FOREACH --------- \n");
            buildStart(asm_file, program->foreach_node.foreach_lowered, current_st, current_ft);
            emit_comment(asm_file, "End lowered foreach node");

            exit_node(asm_file,program, NULL);
            break;
        }


        case NODE_FOR:
        {
            enter_node(asm_file,program);

            Tag *start_tag = program->for_node.label->compound_label.start_tag;
            Tag *end_tag = program->for_node.label->compound_label.end_tag;




            // Initial assignment of for loop iterator
            emit_comment(asm_file, "Initial assignment of for loop iterator");
            buildStart(asm_file, program->for_node.assignment_expr, current_st, current_ft);

            emit_label(asm_file, start_tag->tag_name);
            
            // Condition check 
            emit_comment(asm_file, "Condition check of for loop");
            Operand *condition = buildStart(asm_file, program->for_node.condition_expr, current_st, current_ft);

            // Compare with 0. Use the value sitting in condition.linkedToRegister
            fprintf(asm_file, "    cmp %s, 0\n", condition->linkedToRegister->name);

            // We can unpin now, the condition has already been checked
            // Clear the registers, nothing is longer needed
            unpinRegister(condition->linkedToRegister);
            clearRegisters(gpr);

            // Jump to end for label if condition false
            fprintf(asm_file, "    je %s\n", end_tag->tag_name);

            // Body
            emit_comment(asm_file, "Body");
            buildStart(asm_file, program->for_node.body, current_st, current_ft);

            // Reassignment of the iterator
            emit_comment(asm_file, "Reassignment of the for loop iterator");
            buildStart(asm_file, program->for_node.reassignment_expr, current_st, current_ft);

            // Back to condition check 
            emit_comment(asm_file, "Back to condition check of for loop");
            fprintf(asm_file,"    jmp %s\n\n", start_tag->tag_name);

            // End for loop label
            fprintf(asm_file,"%s:\n", end_tag->tag_name);
            
            exit_node(asm_file,program, NULL);
            break;
        }

        case NODE_IF: 
        {
            enter_node(asm_file,program);

            Label *if_labels = program->if_node.reference_label;

            Tag *end_tag = if_labels->compound_label.end_tag;
            Tag *else_tag = if_labels->compound_label.intermediate_tags[if_labels->compound_label.intermediate_tags_count - 1];
            Tag *next_tag = end_tag;

            /**
             * If there is elseifs, those are the next checks. If there are no elseifs, else is the next check. Else, jump to end if statement
             */
            if (program->if_node.elseif_count > 0){
                next_tag = if_labels->compound_label.intermediate_tags[0];
            }
            else if (program->if_node.else_body != NULL){
                next_tag = else_tag;
            }

            emit_label(asm_file, program->if_node.reference_label->compound_label.start_tag->tag_name);
            // Condition 
            emit_comment(asm_file, "Condition check of if block");
            Operand *condition = buildStart(asm_file, program->if_node.condition_expr, current_st, current_ft);
            
            // Compare with 0. Use the value sitting in condition.linkedToRegister
            fprintf(asm_file, "    cmp %s, 0\n", condition->linkedToRegister->name);

            // We can unpin now, the condition has been checked
            // Clear the registers. Nothing is longer needed
            unpinRegister(condition->linkedToRegister);
            clearRegisters(gpr);

            // Jump if condition false to next check
            fprintf(asm_file,"    je %s\n", next_tag->tag_name); 
            
            // If Body 
            buildStart(asm_file, program->if_node.body, current_st, current_ft);

            // Jump to the absolute end 
            fprintf(asm_file, "    jmp %s\n", end_tag->tag_name);

            // Clear the registers after the if body. No register is longer needed
            clearRegisters(gpr);

            
            // Build for the elseif bodies 
            for (int i = 0; i < program->if_node.elseif_count; i++){
                // Write the label 
                emit_label(asm_file, if_labels->compound_label.intermediate_tags[i]->tag_name);
                // Build for the condition of the elseif bodies
                Operand *elseif_cond = buildStart(asm_file, program->if_node.elseif_nodes[i]->elseif_node.condition_expr, current_st, current_ft);
                fprintf(asm_file, "    cmp %s, 0\n", elseif_cond->linkedToRegister->name);

                clearRegisters(gpr);

                // Update the next check 

                // If it's the last elseif, next is either else or absolute end
                if (i == program->if_node.elseif_count - 1){
                    if (program->if_node.else_body != NULL){
                        next_tag = else_tag;
                    }
                    else{
                        next_tag = if_labels->compound_label.end_tag;
                    }
                }

                // If its not the last else if, then next check is the next elseif 
                else {
                    next_tag = if_labels->compound_label.intermediate_tags[i + 1];
                }

                // Write jump to next check if condition false
                fprintf(asm_file, "    je %s\n", next_tag->tag_name);

                // Build the body
                pushContext(CTX_ELSEIF);
                buildStart(asm_file, program->if_node.elseif_nodes[i]->elseif_node.body, current_st, current_ft);
                clearRegisters(gpr);
                popContext();
                // Jump to the absolute end 
                fprintf(asm_file, "    jmp %s\n", end_tag->tag_name);

            }
            


            // Else Body 
            if (program->if_node.else_body != NULL){
                emit_label(asm_file, else_tag->tag_name);
                pushContext(CTX_ELSE);
                buildStart(asm_file, program->if_node.else_body, current_st, current_ft);
                // Clear the registers after the else body. No register is longer needed
                clearRegisters(gpr);
                popContext();
            }
            
            /**
             * Write the end
             */
            emit_label(asm_file, if_labels->compound_label.end_tag->tag_name);

            exit_node(asm_file,program, NULL);
            break; 
        }

        case NODE_ELSEIF:
        {
            break;
        }

        case NODE_CLASS:
        {
            enter_node(asm_file, program);

            Class *class = lookUpClass(gb_classTable, program->class_node.identifier);

            for (int i = 0; i < program->class_node.funcdefs_count; i++)
            {
                buildStart(asm_file, program->class_node.func_defs[i], class->local_functions->functions[i]->fmi->local_symbols, class->local_functions);
            }

            exit_node(asm_file, program, NULL);
            break;
        }

        case NODE_FUNC_DEF:
        {
            enter_node(asm_file,program);

            if (program->funcdef_node.already_built) { break; }

            Function *f = program->funcdef_node.function;
        
            // Mark the AST as visited
            program->funcdef_node.already_built = 1;
            
            // Build the inner functions first
            buildInnerFunc(asm_file, program->funcdef_node.body, f->fmi->local_symbols, f->fmi->local_functions);

            // Update stack pointer and current function
            updateCFF(f);
            
            // Write the function signature, the label
            emit_label(asm_file, f->fmi->reference_labels->compound_label.start_tag->tag_name);


            // Emit prologue
            emit_prologue(asm_file);

            // Start tracking rsp
            start_tracking_rsp();

            // Load the parameters 
            emit_comment(asm_file, "Loading parameters");
            int params_to_it_until = min(6, f->fmi->param_count);
            int vararg_index = f->fmi->param_count;
            for (int i = 0; i < params_to_it_until && i < vararg_index; i++){
                if (f->fmi->parameters[i]->ptype == P_VARARGS){
                    vararg_index = i;
                    continue;
                }
                Symbol *param = lookUpParameter(f->fmi->local_symbols, i);
                // Where the parameters comes from
                Operand *op_src = createOperand(isTypeUnsignedInt(param->smi->type), request_specific_size_and_type_reg(asm_file, FPR, param->smi->size));
                
                emit_mov_r2s(asm_file, op_src, param->smi->offset, param->smi->size);
            }

            // If we have more than 6 params, calculate their stack position and place them
            // at the correct current stack frame position

            // If the param count is odd, there is an extra 8 bytes we have to consider
            // coming from the sub rsp,8 that the caller executes to ensure 16 byte alignment

            int is_param_count_odd = f->fmi->param_count % 2 != 0 ? 1 : 0;
            if (f->fmi->param_count > 6){
                for (int i = f->fmi->param_count - 1; i >= 6; i--){
                    Symbol *p = lookUpParameter(f->fmi->local_symbols, i);
                    Register *temp = request_specific_size_and_type_reg(asm_file, GPR, p->smi->size);
                    int argument_offset = 8 + 8 * is_param_count_odd + 8 * (f->fmi->param_count - i);
                    fprintf(asm_file, "    mov %s, [rbp + %i]     ; %i th argument\n", temp->name, argument_offset, i + 1); // rbp + (padding and weight) + call + 8 * (N - 6)
                    fprintf(asm_file, "    mov %s [rbp - %i], %s \n", getWordForSize(temp->size),
                            getStackBasePtr() - p->smi->offset, temp->name);
                    unpinRegister(temp);
                }
            }

            emit_comment(asm_file, "Done loading parameters");
            clearRegisters(fpr);
             
            // Instructions 
            buildStart(asm_file, program->funcdef_node.body, f->fmi->local_symbols, f->fmi->local_functions ); 


            // Return label
            emit_label(asm_file, f->fmi->reference_labels->compound_label.end_tag->tag_name);

            // Epilogue
            emit_epilogue(asm_file);

            // Stop tracking rsp
            stop_tracking_rsp();

            exit_node(asm_file,program, NULL);
            break;
        }

        case NODE_FUNC_CALL:
        {
            enter_node(asm_file,program);
            // printf("HERE\n");
            Function *f = program->funccall_node.function;

            emit_comment(asm_file, "Preparing to call function");

            /**    ---------------  COMPUTING THE ARGUMENTS AND PUSHING TO THE STACK ARGUMENTS WHEN WE HAVE MORE THAN 6 --------------------------------- */
            Operand *par;

            int args_pushed = 0;
            int vararg_index = f->fmi->param_count;
            // Build the expressions for the parameters 
            for (int i = 0; i < program->funccall_node.params_count; i++){
                if (vararg_index == f->fmi->param_count && f->fmi->parameters[i]->ptype == P_VARARGS){
                    vararg_index = i;
                }
                 // Push to the stack if >6 parameters or if we are dealing with the varargs
                if (i >= 6 ||  i >= vararg_index){
                    expectRegister(ANY, calculateSizeOfType(program->funccall_node.params_type[i]));
                    par = buildStart(asm_file, program->funccall_node.params_expr[i], current_st, current_ft);
                    clearExpectation();

                    //Ensure we push the 8 byte parent 
                    par->linkedToRegister = getFamilyRegWithSize(par->linkedToRegister, 8);
                    emit_push(asm_file, par->linkedToRegister->name);
                    args_pushed++;
                    unpinRegister(par->linkedToRegister);
                    continue;
                }

                expectRegister(ANY, calculateSizeOfType(program->funccall_node.params_type[i]));
                emit_comment(asm_file, "param");
                par = buildStart(asm_file, program->funccall_node.params_expr[i], current_st, current_ft);
                clearExpectation();
                emit_comment(asm_file, "param finished");

                //Ensure the parameters are stored in the correct FPR registers, otherwise move it in
                if (par->linkedToRegister->type != FPR){
                    Register *fpr_reg = request_specific_size_and_type_reg(asm_file, FPR, calculateSizeOfType(program->funccall_node.params_type[i]));
                    fprintf(asm_file, "    ;Changing to FPR, par was stored in GPR: %s\n", par->linkedToRegister->name);
                    emit_mov_r2r(asm_file, fpr_reg, par->linkedToRegister, fpr_reg->size, par->is_unsigned);
                }

                // Unpin the register used to store the parameter, its now in the right FPR
                unpinRegister(par->linkedToRegister);
            }

            /**    ---------------  END COMPUTING THE ARGUMENTS AND PUSHING TO THE STACK ARGUMENTS WHEN WE HAVE MORE THAN 6 --------------------------------- */

            // Before the call, ensure 16 byte alignment rsp 
            emit_ensure16byte_rsp(asm_file);

            // call func_label
            emit_call(asm_file, f->fmi->reference_labels->compound_label.start_tag->tag_name);
            
            Operand *return_value;
            
            // If return type different than void, there IS a returned value in rax
            if (!isTypeOfKind(f->fmi->rt_type, TYPE_VOID)){
                int size_of_return = calculateSizeOfType(f->fmi->rt_type);
                Operand *rax_operand = createOperand(isTypeUnsignedInt(f->fmi->rt_type), getFamilyRegWithSize(RAX, size_of_return));
                return_value = rax_operand;
                pinRegister(return_value->linkedToRegister);
            }
            // After the call, restore the rsp. Undoes what 'emit_ensure16byte_rsp() did' so:
            emit_restore_rsp(asm_file, args_pushed);
            
            emit_comment(asm_file, "Ended func call node");

            // Exit
            exit_node(asm_file,program, return_value);

            return return_value;
        }

        case NODE_RETURN:
        {
            enter_node(asm_file,program);
            emit_comment(asm_file, "Return node");
            Operand *ret;


            // Get the return label
            Tag *ret_tag = getCurrentFunction()->fmi->reference_labels->compound_label.end_tag;
            if (program->return_node.return_expr == NULL)
            {
                fprintf(asm_file, "    jmp %s\n", ret_tag->tag_name);
                exit_node(asm_file,program, NULL);
                return ret;
            }
            

            // Build the return expresion
            // 13/12 the return expression may NOT be the same size as the declared one. 
            // For example we are allowing return uint8 in a uint32 function, should we allow that?
            ret = buildStart(asm_file, program->return_node.return_expr, current_st, current_ft);          
            /**
             * Return value has to be on RAX, with size matching the defined return type size of the function
             */

            // If we return something
            if (!isTypeOfKind(getCurrentFunction()->fmi->rt_type, TYPE_VOID))
            {
                int size_of_ret = calculateSizeOfType(getCurrentFunction()->fmi->rt_type);

                // If return is not already in RAX's family, move it there 
                if (!are_reg_family(ret->linkedToRegister, RAX))
                {
                    // Request access to RAX, spill to stack if needed
                    // Truncate to match the size of the return type
                    Register *rax = request_specific_reg(asm_file, RAX);
                    Register *sized_rax = getFamilyRegWithSize(rax, size_of_ret);
                    emit_comment(asm_file, "Ensure return is in rax");

                    ret->linkedToRegister = sized_rax;

                }

                emit_mov_r2r(asm_file, getFamilyRegWithSize(ret->linkedToRegister, size_of_ret), ret->linkedToRegister, size_of_ret, ret->is_unsigned);
            }

            // Pin the register associated to the return value
            pinRegister(ret->linkedToRegister);

            // Jump to return label
            fprintf(asm_file, "    jmp %s\n", ret_tag->tag_name);
            emit_comment(asm_file, "Ended return node");
            exit_node(asm_file,program, ret);

            return ret;
        }
        
        
        case NODE_ARRAY_INIT:
        {
            enter_node(asm_file,program);

            emit_comment(asm_file, "Array init");
            // Get the expected register 
            Symbol *arr = lookUpSymbol(current_st, context.current_scope, program->array_init_node.arr_name);
            Type *element_type = arr->smi->type->arrayType.element_type;
            int size_of_every_element = calculateSizeOfType(element_type); 

            Register *first_element = NULL;
            
            /**
             * 28/11: Later change to > 16. Remember to use XMM registers to mov < 16 byte data
             */
            if (size_of_every_element > 8)
            {
                for (int i = 0; i < program->array_init_node.element_count; i++)
                {

                    emit_comment(asm_file, "Copying");

                    fprintf(asm_file, "    lea rdi, [rbp - %i]\n", getStackBasePtr() - arr->smi->offset);
                    fprintf(asm_file, "    add rdi, %i \n", i * size_of_every_element);  

                    // lea rax, [rbp - offset] -> loads into rax the address of ith element
                    Operand *result = with_lvalue_context(asm_file, program->array_init_node.elements[i], current_st, current_ft); 

                    fprintf(asm_file, "    lea rsi, [%s]\n", result->linkedToRegister->name);
                    fprintf(asm_file, "    mov rcx, %i \n", size_of_every_element);
                    fprintf(asm_file, "    rep movsb \n");

                    // Link a register to the base of the array (first element)
                    if (i == 0)  { first_element = result->linkedToRegister; }

                    // Unpin the register, it's already been consumed
                    unpinRegister(result->linkedToRegister);
                }
            }

            // normal mov
            else 
            {
                // Push an expectation, every element of the array must use the same size registers
                expectRegister(GPR, size_of_every_element);
                char *byte_word = getWordForSize(size_of_every_element);

                for (int i = 0; i < program->array_init_node.element_count; i++)
                {
                    Operand *result = buildStart(asm_file, program->array_init_node.elements[i], current_st, current_ft);

                    emit_mov_r2s(asm_file, result,  i * size_of_every_element + arr->smi->offset, size_of_every_element);
                    if (i == 0) 
                    {
                        first_element = result->linkedToRegister;
                    }
                    // Unpin the register, it's already been consumed
                    unpinRegister(result->linkedToRegister);
                }

                // Clear the expectation, we are finished
                clearExpectation();
            }

            
            // Pin the register holding the base address of the array
            pinRegister(first_element);

            emit_comment(asm_file, "Array init end");
            Operand *result = createOperand(isTypeUnsignedInt(element_type), first_element);
            exit_node(asm_file,program, result);

            return result;
        }

        case NODE_SUBSCRIPT: 
        {

            enter_node(asm_file,program);

            emit_comment(asm_file, "Array subscript");



            Operand *base;

            //printf("Computing base\n");
            base = with_lvalue_context(asm_file, program->subscript_node.base, current_st, current_ft);
            //printf("Finished\n");

            if (isTypeOfKind(program->subscript_node.base_type, TYPE_STR) || isTypeOfKind(program->subscript_node.base_type, TYPE_PTR))
            {
                emit_comment(asm_file, "PTR/STR subscript");
                emit_mov_a2r(asm_file, base, base, 8); // mov rax, [rax]
            }

            /**
             * Compute the RVALUE (value) index on an 8 byte register
             */
            emit_comment(asm_file, "Computing the index");
            Operand *index = with_rvalue_context(asm_file, program->subscript_node.index, ANY, program->subscript_node.index_size, current_st, current_ft);
            



            if (index->linkedToRegister->size < 4)
            {
                
                emit_mov_r2r(asm_file, getParent64(index->linkedToRegister), index->linkedToRegister, 8, index->is_unsigned);
                index->linkedToRegister = getParent64(index->linkedToRegister);
            }


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
            //fprintf(asm_file, "    add %s, %s \n", base->linkedToRegister->name, index->linkedToRegister->name);
            promote_bigger_reg_size(asm_file, &base->linkedToRegister, &index->linkedToRegister, index->is_unsigned, base->is_unsigned);
            emit_add(asm_file, base->linkedToRegister, index->linkedToRegister);
        
            // Unpin the register holding the index value, it's been consumed
            unpinRegister(index->linkedToRegister);

            // If we are in LVALUE context, we want the address
            Operand *op_result = createOperand(base->is_unsigned, base->linkedToRegister);
            // If computing RVALUE, we want the value, not the address 
            if (getCurrentWorkingContext() == COMPUTING_RVALUE || getCurrentWorkingContext() == COMPUTING_IMMEDIATE) 
            {

                // Move into it the value sitting at the address we calculated
                emit_mov_a2r(asm_file, op_result, base, program->subscript_node.element_size);

                // Unpin the register holding the address
                unpinRegister(base->linkedToRegister);

                // Make sure op_result is of the size specified
                op_result->linkedToRegister = getFamilyRegWithSize(op_result->linkedToRegister, program->subscript_node.element_size);
            }

            // Pin the register holding the result, whether is the address or the value
            pinRegister(op_result->linkedToRegister);

            emit_comment(asm_file, "Array subscript end");
            //printf("ARRAY SUBSCRIPT END RESULT = %s\n", op_result.linkedToRegister->name);
            exit_node(asm_file,program, op_result);

            return op_result;
           
        }

        case NODE_METHOD_DISPATCH:
        {
            enter_node(asm_file, program);
            emit_comment(asm_file, "Method dispatch");

            Class *c = lookUpClass(gb_classTable, program->method_dispatch.base_type->udt.name);

            Operand *md = buildStart(asm_file, program->method_dispatch.md_lowered, current_st, c->local_functions);

            exit_node(asm_file, program, md);
            emit_comment(asm_file, "Finish method dispatch");

            return md;
        }

        case NODE_PTR_METHOD_DISPATCH:
        {
            enter_node(asm_file, program);
            emit_comment(asm_file, "Ptr method dispatch");

            Type *class_type = program->ptr_method_dispatch.base_type->pointerType.pointed_type;
            Class *c = lookUpClass(gb_classTable, class_type->udt.name);

            Operand *ptr_md = buildStart(asm_file, program->ptr_method_dispatch.ptrmd_lowered, current_st, c->local_functions);
            exit_node(asm_file, program, ptr_md);
            emit_comment(asm_file, "Finish ptr method dispatch");
            return ptr_md;
        }

        case NODE_FIELD_ACCESS:
        {
            enter_node(asm_file,program);
            emit_comment(asm_file, "Field access");

            Symbol *field;
            /**
             * Retrieve the type the instance is of
             */
            switch(program->field_access_node.type->udt.udt_kind)
            {
                case UDT_CLASS:
                {
                    Class *c = lookUpClass(gb_classTable, program->field_access_node.type->udt.name);
                    field = lookUpField(c->local_symbols, program->field_access_node.field_name);
                    break;
                }
                case UDT_OBJECT:
                {
                    Object *o = lookUpObject(gb_objectTable, program->field_access_node.type->udt.name);
                    field = lookUpField(o->local_symbols, program->field_access_node.field_name);
                    break;
                }
            }

            /**
             * Compute the LVALUE (address) base
             */
            Operand *base = with_lvalue_context(asm_file, program->field_access_node.base, current_st, current_ft);

            /**
             * Add the field offset to the base
             */
            fprintf(asm_file, "    add %s, %i \n", base->linkedToRegister->name, field->smi->offset);   

            // If we are in LVALUE context, we want the address
            Operand *op_result = createOperand(isTypeUnsignedInt(field->smi->type), base->linkedToRegister);
            // If computing RVALUE, we want the value, not the address 
            if (getCurrentWorkingContext() == COMPUTING_RVALUE || getCurrentWorkingContext() == COMPUTING_IMMEDIATE)
            {
                // Request a GPR register big enough to hold the field data type
                //op_result->linkedToRegister = request_specific_size_reg(asm_file, field->size);
                
                // Move into it the value sitting at the address we calculated
                emit_mov_a2r(asm_file, op_result, base, field->smi->size);
                op_result->linkedToRegister = getFamilyRegWithSize(base->linkedToRegister, field->smi->size);
                // Unpin the register holding the address, is no longer needed
                unpinRegister(base->linkedToRegister);
            }

            // Pin the register holding the result, whether is the address or the value
            pinRegister(op_result->linkedToRegister);
            


            emit_comment(asm_file, "Field access end");
            exit_node(asm_file,program, op_result);

            return op_result;
        }

        case NODE_PTR_FIELD_ACCESS:
        {
            enter_node(asm_file,program);
            emit_comment(asm_file, "Ptr field access");

            /**
             * Calculate the base address (pointer)
             * Dereference so we get the address of the pointed object
             */
            Operand *base = with_lvalue_context(asm_file, program->ptr_field_access_node.base, current_st, current_ft);
            emit_mov_a2r(asm_file, base, base, base->linkedToRegister->size);

            Symbol *field;
            /**
             * Retrieve the type the instance is of
             */
            switch(program->ptr_field_access_node.type->udt.udt_kind)
            {
                case UDT_CLASS:
                {
                    Class *c = lookUpClass(gb_classTable, program->ptr_field_access_node.type->udt.name);
                    field = lookUpField(c->local_symbols, program->ptr_field_access_node.field_name);
                    printf("Here\n");
                    break;
                }
                case UDT_OBJECT:
                {
                    Object *o = lookUpObject(gb_objectTable, program->ptr_field_access_node.type->udt.name);
                    field = lookUpField(o->local_symbols, program->ptr_field_access_node.field_name);
                    break;
                }
                default:{
                    fprintf(stderr, "Idonotknow\n");
                    exit(1);
                }
            }
            /**
             * Add the field offset to the base
             */
            fprintf(asm_file, "    add %s, %i \n", base->linkedToRegister->name, field->smi->offset);

            // If we are in LVALUE context, we want the address
            Operand *op_result = createOperand(base->is_unsigned, base->linkedToRegister);
            // If computing RVALUE, we want the value, not the address 
            if (getCurrentWorkingContext() == COMPUTING_RVALUE || getCurrentWorkingContext() == COMPUTING_IMMEDIATE)
            {            

                // Move into it the value sitting at the address we calculated
                emit_mov_a2r(asm_file, base, base, field->smi->size);

                // Unpin the register holding the address, no longer needed
                unpinRegister(base->linkedToRegister);

                // Make sure op_result is of the size specified
                op_result->linkedToRegister = getFamilyRegWithSize(op_result->linkedToRegister, field->smi->size);
            }

            // Pin the register holding the result, whether is the address or the value
            pinRegister(op_result->linkedToRegister);


            emit_comment(asm_file, "Ptr field access end");
            exit_node(asm_file,program, op_result);


            return op_result;
        }

        case NODE_ASSIGNMENT:
        {
            enter_node(asm_file,program);


            /**
             * Do nothing if the Context is NONE. This means the assignment is for a global variable
             * which was done in buildData(). Dont build again for it
             */
            if (getCurrentContext() == CTX_NONE)
            {
                return NULL;
            }
            

            int size = calculateSizeOfType(program->assignment_node.type);


            fprintf(asm_file, "    ;Building assignment expression \n");

            Operand *rvalue = with_rvalue_context(asm_file, program->assignment_node.expression, GPR, size, current_st, current_ft );

            Symbol *identifier = lookUpSymbol(current_st, context.current_scope, program->assignment_node.identifier);
            /**
             * If the identifier is not of type array, then we must move it onto its right position in the stack.
             * For an array type, this was done in NODE_ARRAY_INIT
             * 17/12 --> not really, we could have uint16[3] y = x; with x being uint16[3]
             */

            if (!isTypeOfKind(identifier->smi->type, TYPE_ARRAY))
            {
                emit_mov_r2s(asm_file, rvalue, identifier->smi->offset, identifier->smi->size);
            }

            // Unpin rvalue, its no longer needed 
            unpinRegister(rvalue->linkedToRegister);
            exit_node(asm_file,program, NULL);
            return NULL;
        }

        case NODE_REASSIGNMENT:
        {

            enter_node(asm_file,program);


            if (getCurrentContext() == CTX_NONE)
            {
                return NULL;
            }

            Operand *lvalue;
            Operand *rvalue;

            emit_comment(asm_file, "Reassignment");

            emit_reassign(asm_file, lvalue, rvalue, &program->reassignment_node, current_st, current_ft);

            exit_node(asm_file,program, NULL);
            
            return NULL;
        }

        case NODE_DECLARATION:
        {
            enter_node(asm_file,program);
            // Done in the first pass 
            if (getCurrentContext() == CTX_NONE)
            {
                return NULL;
            }

            exit_node(asm_file,program, NULL);
            return NULL;
        }

        /**
         * NOT CHANGED
         */
        case NODE_BINARY_OP:
        {
            

            enter_node(asm_file,program);

            /**
             * Compute the binary operation depth. The deepest branch will be evaluated first so we dont run out of registers
             */
            int left_depth = compute_binaryOperationDepth(program->binary_op_node.left, 1);
            int right_depth = compute_binaryOperationDepth(program->binary_op_node.right, 1);
            
            
            Register *temp = NULL;

            Operand *left = NULL;
            Operand *right = NULL;

            BinOPCode binOperation = strToBinOPCode(program->binary_op_node.op); 

            int atLeastOneSigned;
            /**
             * Build for the deepest branch of the binary operation first IF ITS NOT A SHORT-CIRCUIT OP
             */
            if (binOperation != AND && binOperation != OR){
                emit_binop_deepest(asm_file, &program->binary_op_node, &left, &right, current_st, current_ft);

                
                //printf("Before promoting. OP = %s. Left = %s, right = %s\n", program->binary_op_node.op, left.linkedToRegister->name, right.linkedToRegister->name);
                promote_bigger_reg_size(asm_file, &left->linkedToRegister, &right->linkedToRegister, right->is_unsigned, left->is_unsigned);
                //printf("After promoting. OP = %s. Left = %s, right = %s\n", program->binary_op_node.op, left.linkedToRegister->name, right.linkedToRegister->name);

                // Check whether they are signed or not
                int atLeastOneSigned = !left->is_unsigned || !right->is_unsigned;
            }


            switch(binOperation)
            {
                case ADD:
                {
                    fprintf(asm_file, "    add %s, %s\n", left->linkedToRegister->name, right->linkedToRegister->name);
                    break;
                }

                case MUL: 
                {
                    emit_mul(asm_file, left, right);

                    Register *reg = (left->linkedToRegister->size == 1 && right->linkedToRegister->size == 1)? getFamilyRegWithSize(RAX,2) : left->linkedToRegister;

                    Operand *result = createOperand(!atLeastOneSigned, reg);

                    pinRegister(reg);
                    exit_node(asm_file, program, result);
                    return result;
                    break;
                }


                case SUB: 
                {
                    fprintf(asm_file, "    sub %s , %s\n",left->linkedToRegister->name, right->linkedToRegister->name); 
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
                    emit_div(asm_file, left, right);

                    /**
                     * Pin EAX 
                     */
                    pinRegister(EAX);

                    // Return the result in EAX
                    Operand *result = createOperand(!atLeastOneSigned, EAX);
                    //busyRegister(result.linkedToRegister);

                    exit_node(asm_file,program, result);
                    return result;
                    break;
                }

                case MOD: 
                {
                    emit_div(asm_file, left, right);

                    /**
                     * Pin EDX
                     */
                    pinRegister(EDX);

                    // Return the result in EDX
                    Operand *result = createOperand(!atLeastOneSigned, EDX);

                    //busyRegister(result.linkedToRegister);

                    exit_node(asm_file,program, result);
                    return result;
                    break;                   
                }

                // Shift left n bits -> multiply by 2^n
                // n must be in CL for 8/16/32 bits shifts
                // for 64 bit shifts, n can be in any 8 bit register
                case SHIFT_L:
                {
                    // Mov from right to CL
                    if (!are_reg_family(right->linkedToRegister, RCX))
                    {
                        Register *rcx = request_specific_reg(asm_file, RCX);
                        emit_mov_r2r(asm_file, getFamilyRegWithSize(rcx, 1), right->linkedToRegister, 1, left->is_unsigned);
                        // Unpin right, count is in CL now
                        unpinRegister(right->linkedToRegister);
                    }

                    // Only need shl for left shifts
                    fprintf(asm_file, "    shl %s, %s\n", left->linkedToRegister->name, getFamilyRegWithSize(RCX, 1)->name);
                    break;
                }

                case SHIFT_R:
                {

                    // Mov from right to CL
                    if (!are_reg_family(right->linkedToRegister, RCX))
                    {
                        Register *rcx = request_specific_reg(asm_file, RCX);
                        emit_mov_r2r(asm_file, getFamilyRegWithSize(rcx, 1), right->linkedToRegister, 1, left->is_unsigned);
                        // Unpin right, count is in CL now
                        unpinRegister(right->linkedToRegister);
                    }
                    
                    char *shift_op = (left->is_unsigned)? "shr" : "sar";
                    fprintf(asm_file, "    %s %s, %s\n", shift_op, left->linkedToRegister->name, getFamilyRegWithSize(RCX, 1)->name);
                    break;
                }

                case GT: 
                {
                    Register *leftEightBitRegister = getFamilyRegWithSize(left->linkedToRegister,1);
                    fprintf(asm_file, "    cmp %s, %s\n", left->linkedToRegister->name, right->linkedToRegister->name);

                    char *setg_op = (left->is_unsigned && right->is_unsigned)? "seta" : "setg"; 

                    fprintf(asm_file, "    %s %s\n", setg_op, leftEightBitRegister->name );
                    left->linkedToRegister = leftEightBitRegister;
                    break;
                }

                case LT: 
                {
                    Register *leftEightBitRegister = getFamilyRegWithSize(left->linkedToRegister,1);
                    fprintf(asm_file, "    cmp %s, %s\n", left->linkedToRegister->name, right->linkedToRegister->name);

                    char *setl_op = (left->is_unsigned && right->is_unsigned)? "setb" : "setl"; 

                    fprintf(asm_file, "    %s %s\n", setl_op, leftEightBitRegister->name );
                    left->linkedToRegister = leftEightBitRegister;
                    break;               
                }

                case GTEQ:
                {
                    Register *leftEightBitRegister = getFamilyRegWithSize(left->linkedToRegister,1);
                    fprintf(asm_file, "    cmp %s, %s\n", left->linkedToRegister->name, right->linkedToRegister->name);

                    char *setge_op = (left->is_unsigned && right->is_unsigned)? "setae" : "setge";
                    fprintf(asm_file, "    %s %s\n", setge_op, leftEightBitRegister->name );
                    left->linkedToRegister = leftEightBitRegister;
                    break;
                }

                case LTEQ:
                {
                    Register *leftEightBitRegister = getFamilyRegWithSize(left->linkedToRegister,1);
                    fprintf(asm_file, "    cmp %s, %s\n", left->linkedToRegister->name, right->linkedToRegister->name);

                    char *setl_op = (left->is_unsigned && right->is_unsigned)? "setbe" : "setle";
                    fprintf(asm_file, "    %s %s\n", setl_op, leftEightBitRegister->name);
                    left->linkedToRegister = leftEightBitRegister;
                    break;
                }

                case EQ:
                {
                    Register *leftEightBitRegister = getFamilyRegWithSize(left->linkedToRegister,1);
                    fprintf(asm_file, "    cmp %s, %s\n", left->linkedToRegister->name, right->linkedToRegister->name);
                    fprintf(asm_file, "    sete %s\n", leftEightBitRegister->name);
                    left->linkedToRegister = leftEightBitRegister;
                    break;
                }

                case NEQ:
                {
                    Register *leftEightBitRegister = getFamilyRegWithSize(left->linkedToRegister,1);
                    fprintf(asm_file, "    cmp %s, %s\n", left->linkedToRegister->name, right->linkedToRegister->name);
                    fprintf(asm_file, "    setne %s\n", leftEightBitRegister->name);
                    left->linkedToRegister = leftEightBitRegister;
                    break;
                }

                case BIT_AND:
                {
                    fprintf(asm_file, "    and %s,%s\n", left->linkedToRegister->name, right->linkedToRegister->name);
                    break;
                }

                case BIT_OR:
                {
                    fprintf(asm_file, "    or %s,%s\n", left->linkedToRegister->name, right->linkedToRegister->name);
                    break;
                }
                
                // logical and --> Shortcircuit
                case AND:
                {
                    // Build left first
                    left = buildStart(asm_file, program->binary_op_node.left, current_st, current_ft);
                    pinRegister(left->linkedToRegister);

                    Register *leftEightBitRegister = getFamilyRegWithSize(left->linkedToRegister,1);
                    // Test whether the first operand is zero 
                    fprintf(asm_file, "    test %s, %s\n", left->linkedToRegister->name, left->linkedToRegister->name);
                    // Set if non equal 
                    fprintf(asm_file, "    setne %s\n", leftEightBitRegister->name );

                    // Skip right one if left is zero
                    Tag *skipRightTag = makeTag("skip_right_%i", context.skip_right_id++);
                    fprintf(asm_file, "    jz %s\n", skipRightTag->tag_name);
                    
                    // Build right side now 
                    right = buildStart(asm_file, program->binary_op_node.right, current_st, current_ft);
                    pinRegister(right->linkedToRegister);
                    Register *rightEightBitRegister = getFamilyRegWithSize(right->linkedToRegister, 1);

                    // Test whether the second operand is zero 
                    fprintf(asm_file, "    test %s, %s\n", right->linkedToRegister->name, right->linkedToRegister->name);
                    // Set if not equal
                    fprintf(asm_file, "    setne %s\n", rightEightBitRegister->name );

                    // Perform and operation 
                    fprintf(asm_file, "    and %s, %s\n", leftEightBitRegister->name, rightEightBitRegister->name);
                    left->linkedToRegister = leftEightBitRegister;

                    // Skip right label
                    emit_label(asm_file, skipRightTag->tag_name);
                    break;
                }
                
                // logical or ---> Shortcircuit
                case OR: 
                {
                    // Build left first
                    left = buildStart(asm_file, program->binary_op_node.left, current_st, current_ft);
                    pinRegister(left->linkedToRegister);
                    Register *leftEightBitRegister = getFamilyRegWithSize(left->linkedToRegister,1);

                    // Test whether the first operand is zero 
                    fprintf(asm_file, "    test %s, %s\n", left->linkedToRegister->name, left->linkedToRegister->name);
                    // Set if non equal 
                    fprintf(asm_file, "    setne %s\n", leftEightBitRegister->name );

                    // Skip right one if left is one
                    Tag *skipRightTag = makeTag("skip_right_%i", context.skip_right_id++);
                    fprintf(asm_file, "    jnz %s\n", skipRightTag->tag_name);

                    // Build right side now 
                    right = buildStart(asm_file, program->binary_op_node.right, current_st, current_ft);
                    pinRegister(right->linkedToRegister);
                    Register *rightEightBitRegister = getFamilyRegWithSize(right->linkedToRegister, 1);

                    // Test whether the second operand is zero 
                    fprintf(asm_file, "    test %s, %s\n", right->linkedToRegister->name, right->linkedToRegister->name);
                    // Set if not equal
                    fprintf(asm_file, "    setne %s\n", rightEightBitRegister->name );

                    // Perform and operation 
                    fprintf(asm_file, "    or %s, %s\n", leftEightBitRegister->name, rightEightBitRegister->name);
                    left->linkedToRegister = leftEightBitRegister;

                    // Skip right label
                    emit_label(asm_file, skipRightTag->tag_name);

                    break;                
                }

                default: 
                {
                    fprintf(stderr, "Unsupported binary operation to build. Got: %i \n", binOperation );
                    exit(1);
                }
            
            }
            
            /**
             * Restore potential spilling back to the state it was at when we enter this node
             */

            unpinRegister(right->linkedToRegister); 

            //int new_spilled_count = rqOffice.spill_count;
            //collect_spilled_regs_until(asm_file, new_spilled_count - already_spilled_count);

            //printf("Returning left in %s\n", left.linkedToRegister->name);


            Operand *result = createOperand(atLeastOneSigned, left->linkedToRegister);
            exit_node(asm_file,program, result);
            return result;
        }

        case NODE_POSTFIX_OP:
        {
            enter_node(asm_file,program);


            PostfixOPCode postfixOPCode = strToPostfixOPCode(program->postfix_op_node.op);

            /**
             * Get the address we wanna apply the postfix op at
             */
            Operand *operand = with_lvalue_context(asm_file, program->postfix_op_node.left, current_st, current_ft);

            /**
             * Request a specific GPR big enough to hold the data we are applying the postfix op to
             */
            Register *temp = request_specific_size_reg(asm_file, program->postfix_op_node.size_of_operand);
            Operand *op_temp = createOperand( operand->is_unsigned, temp);
            /**
             * Move into that register the value sitting at the address we wanna aply the postfix at,
             * so the oncoming operations use the value BEFORE applying the postfix op
             */

            emit_mov_a2r(asm_file, op_temp, operand, program->postfix_op_node.size_of_operand);

            /**
             * Check for whether its a pointer or not. 
             * Different rules apply
             */
            int is_ptr = isTypeOfKind(program->postfix_op_node.type, TYPE_PTR);

            emit_postfix(asm_file, is_ptr, program->postfix_op_node.size_of_operand, *operand, postfixOPCode);

            /**
             * Unpin operand, no longer needed what we care about is in temp
             */
            unpinRegister(operand->linkedToRegister);

            // If is stdalone, unpin temp too, nobody needs it
            if (program->is_stmt == 1)
            {
                unpinRegister(temp);
            }

            exit_node(asm_file,program, op_temp);
            return op_temp;
        }

        case NODE_UNARY_OP:
        {
            enter_node(asm_file,program);

            /**
             * Result is gonna be here
             */
            Operand *operand;

            /**
             * Get the unary op code
             */
            UnaryOPCode unaryOPCode = strToUnaryOPCode(program->unary_op_node.op);
            switch(unaryOPCode)
            {
                case UNARY_MINUS:
                {
                    operand = buildStart(asm_file, program->unary_op_node.right, current_st, current_ft);
                    fprintf(asm_file, "    neg %s\n",  operand->linkedToRegister->name);

                    //Not sure of operand.is_unsigned = 0 now
                    operand->is_unsigned = 0;
                    break;
                }

                case UNARY_NOT:
                {
                    operand = buildStart(asm_file, program->unary_op_node.right, current_st, current_ft);
                    fprintf(asm_file, "    test %s, %s\n", operand->linkedToRegister->name, operand->linkedToRegister->name);

                    Operand result = {.linkedToRegister = request_specific_size_reg(asm_file, 1), .is_unsigned = 1};

                    fprintf(asm_file, "    setz %s\n", result.linkedToRegister->name);
                    operand->is_unsigned = result.is_unsigned; 
                    operand->linkedToRegister = result.linkedToRegister;
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

                    if (getCurrentWorkingContext() != COMPUTING_LVALUE){
                        // Get the value from the address
                        emit_mov_a2r(asm_file, operand, operand, program->unary_op_node.size_of_operand);
                        operand->linkedToRegister = getFamilyRegWithSize(operand->linkedToRegister, program->unary_op_node.size_of_operand);
                    }
                    else {

                        emit_mov_a2r(asm_file, operand, operand, 8);
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
            pinRegister(operand->linkedToRegister);
            exit_node(asm_file,program, operand);
            return operand;
        }

        case NODE_IDENTIFIER:
        {

            enter_node(asm_file,program);

            /**
             * Look up the identifier
             */
            Symbol *s = lookUpSymbol(current_st, context.current_scope, program->identifier_node.name );
            
            Operand *identifier = load_identifier(asm_file, s); 
            pinRegister(identifier->linkedToRegister);
            exit_node(asm_file,program, identifier);
            return identifier;
        }

        case NODE_NUMBER: 
        {
            enter_node(asm_file,program);

            Operand *number = load_imm(asm_file, calculateSizeOfType(program->number_node.number_type), program);

            pinRegister(number->linkedToRegister);

            exit_node(asm_file,program, number);

            return number;
        }

        case NODE_CHAR:
        {
            enter_node(asm_file,program);
            
            Operand *character = load_imm(asm_file, 1, program);
            pinRegister(character->linkedToRegister);

            exit_node(asm_file,program, character);
            return character;
        }

        case NODE_STR:
        {
            enter_node(asm_file, program);

            Operand *str = load_imm(asm_file, 8, program);
            pinRegister(str->linkedToRegister);

            exit_node(asm_file, program, str);
            return str;
        }

        case NODE_BOOL:
        {
            enter_node(asm_file,program);
            
            Operand *b = load_imm(asm_file, 1, program);
            pinRegister(b->linkedToRegister);

            exit_node(asm_file,program, b);
            
            return b;
        }

        
        default:
        {
            break;
        }
    }
}
