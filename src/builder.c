


#include "../include/builder.h"
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <inttypes.h>


Context bt_context = {
    .current_scope = 0,
    .if_block_id = 0,
    .else_block_id = 0,
    .for_block_id = 0,
    .foreach_block_id = 0,
    .while_block_id = 0,
    .function_block_id = 0,
    .func_call_id = 0,
    .ctx_type_stack = {CTX_NONE},
};




RegisterTable *gpr = NULL;
RegisterTable *fpr = NULL;
Matcher *matcher = NULL;

<<<<<<< Updated upstream
=======
CurrentFrameAndFunction CFF = { .stack_base_ptr = 0, .f = NULL };


///////////////////// UTILS

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

Register *getSubRegisterAtIndex(Register *root, int index)
{
    if (index >= 3 || index < 0)
    {
        fprintf(stderr, "Getting subregister at index: %s, makes no sense\n", index);
    }

    if (root->size == 1)
    {
        fprintf(stderr, "There is no subregister for root register: %s, at index: %i \n", root->name, index);
        exit(1);
    }



    return root->subRegisters[index];
}

Register *getSubRegisterWithSize(Register *root, int size)
{
    if (root->size == 1)
    {
        fprintf(stderr, "There is no subregister for root register: %s, with size: %i \n", root->name, size);
        exit(1);
    }

    for (int i = 0; i < 3; i++)
    {
        if (root->subRegisters[i]->size == size)
        {
            return root->subRegisters[i];
        }
    }
}

Register *getFamilyRegWithSize(Register *root, int size)
{
    Register *parent = (root->parent64Register == NULL)? root : root->parent64Register;

    if (parent->size == size) { return parent; }

    for (int i = 0; i < 3; i++)
    {
        if (parent->subRegisters[i]->size == size) { return parent->subRegisters[i]; }
    }

    fprintf(stderr, "There is no family register for root register: %s, with size: %i \n", root->name, size);

}

int are_reg_family(Register *a, Register *b)
{
    if (a == b) { return 1; }

    Register *parent = (a->parent64Register == NULL) ? a : a->parent64Register;

    for (int i = 0; i < 3; i++)
    {
        if (parent->subRegisters[i] == b) { return 1; }
    }

    return 0;
}


void promote_smaller_reg_size(Register **regA, Register **regB)
{
    if ((*regA)->size != (*regB)->size)
    {
        int min_size = min((*regA)->size, (*regB)->size);

        *regA = getFamilyRegWithSize(*regA, min_size);
        *regB = getFamilyRegWithSize(*regB, min_size);
    }
}

void promote_bigger_reg_size(Register **regA, Register **regB)
{
    if ((*regA)->size != (*regB)->size)
    {
        int min_size = max((*regA)->size, (*regB)->size);

        *regA = getFamilyRegWithSize(*regA, min_size);
        *regB = getFamilyRegWithSize(*regB, min_size);
    }
}


/* Helper: remove whitespace – works on a copy only*/
void remove_whitespace_copy(char *dst, const char *src, size_t max_len)
{
    while (*src && max_len > 1) {
        if (!isspace((unsigned char)*src)) {
            *dst++ = *src;
            max_len--;
        }
        src++;
    }
    *dst = '\0';
}

// Count leading "ptr" tokens and return the pointer level + advance pointer to base type
int count_ptr_levels(const char **p)
{
    const char *t = *p;
    int level = 0;

    while (1) {
        /* skip possible leading whitespace (should be none after remove_whitespace_copy) */
        while (isspace((unsigned char)*t)) t++;

        if (strncmp(t, "ptr", 3) != 0) break;

        t += 3;
        level++;
    }
    *p = t;
    return level;
}



///////////////////// UTILS









////////////////////// EMIT

// Writes the comment to the asm file with \n at the end
void emit_comment(FILE *asm_file, char *comment)
{
    fprintf(asm_file, "    ;%s\n", comment);
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
        if (gpr->registers[0]->is_being_used)
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
void emit_mul_imm(FILE *asm_file, Operand left, int number, int size)
{
    fprintf(asm_file, "    imul %s, %s, %i \n", left.linkedToRegister->name, left.linkedToRegister->name, number);
}


// ----------------------------------- REASSIGNMENTS ----------------------------------------------------

// Only call this function from within emit_reassign()
void emit_memcpy(FILE *asm_file, Register *dst, Register *src, int byte_size)
{
    fprintf(asm_file, "    lea rdi, [%s]\n", dst->name);
    fprintf(asm_file, "    lea rsi, [%s]\n", src->name);
    fprintf(asm_file, "    mov rcx, %i\n", byte_size);
    fprintf(asm_file, "    rep movsb\n");
}

// Only call this function from within emit_reassign()
void emit_ptr_reassign(FILE *asm_file, Register *left, Register *right, int pointed_size, TokenType REASSIGN_TOKEN)
{
    switch(REASSIGN_TOKEN)
    {
        case TOKEN_ASSIGN:
        {
            fprintf(asm_file, "    mov %s [%s], %s \n", 
                getWordForSize(right->size), left->name, right->name);
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
            fprintf(asm_file, "    mov %s %s, [%s] \n", 
                getWordForSize(right->size), left->name, right->name);
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

// Free to call .
// Parameters:
//  - left: Register that holds the address to write to
//  - right: Register that holds the address we read from
void emit_reassign(FILE *asm_file, Register *left, Register *right, ReassignmentNode *reassignment_node)
{
    // If big struct, mempcy
    if (reassignment_node->size > 8) { return emit_memcpy(asm_file, left, right, reassignment_node->size); }

    // We want the value stored at 'right', not the address 
    //fprintf(asm_file, "    mov %s, [%s]\n", right->name, right->name);

    // Pointer reassignment
    if (is_type_ptr(reassignment_node->type)) { return emit_ptr_reassign(asm_file, left, right, size_of_type(resolve_final_ptr_type(reassignment_node->type,1)), reassignment_node->op);  } 

    // Scalar reassignment
    emit_scalar_reassign(asm_file, left, right, reassignment_node->size, reassignment_node->op);

}


// ----------------------------------- REASSIGNMENTS ----------------------------------------------------





//**  -------------------------------------- EMIT --------------------------------------------- **//

void updateCFF(Function *f)
{
    CFF.f = f;
    // RSP has to be subtracted enough to account for the weight of all variables AND the padding in between them
    //printf("FUNCTION: %s, has weight: %i, and padding: %i \n", f->identifier, f->local_symbols->weight, f->local_symbols->padding);
    CFF.stack_base_ptr = align_rsp(f->local_symbols->weight + f->local_symbols->padding);

    //printf("Total weight and padding for function %s : %i + %i = %i\n", f->identifier,f->local_symbols->weight, f->local_symbols->padding,
    //        f->local_symbols->weight + f->local_symbols->padding);
}

Function *getCurrentFunction() { return CFF.f; }

int getStackBasePtr() { return CFF.stack_base_ptr; }


void enter_node(ASTNode *node)
{
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
        printf("--- CLEARED GPR AND FPR \n");
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
    expectRegister(GPR, 8);
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


int align_rsp(int locals_weight)
{
    printf("Aligning rsp, we've entered a function with locals weight = %i \n", locals_weight);
    int total = (locals_weight + 8 + 15) & ~15; // Round up weight + 8 to multiple of 16
    int align_size = total - 8; // Subtract push rbp
    if (align_size < 8) align_size = 8;

    printf("Align_size = %i \n", align_size);

    return align_size;
}


void initWorkingContext()
{
    workingContext.typeStack[0] = COMPUTING_NOTHING;
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
        case COMPUTING_NOTHING: 
        case COMPUTING_RVALUE: return "mov";
        case COMPUTING_LVALUE: return "lea";
    }
}
>>>>>>> Stashed changes

void initMatcher()
{
    matcher = malloc(sizeof(Matcher));
    matcher->expecation_to_match = malloc(sizeof(Expectation));
    matcher->expecation_to_match->size = 0;
    matcher->expecation_to_match->registerType = ANY;
    matcher->empty = 1;
}

Expectation *getCurrentExpectation()
{
    return matcher->expecations_to_match[matcher->current_expectation_index];
}


void expectRegister(RegisterType registerType, int size)
{
    if (!matcher->empty)
    {
        fprintf(stderr, "Cannot expect more than one expectations at a time. \n");
        exit(1);
    }

<<<<<<< Updated upstream
    matcher->expecation_to_match->registerType = registerType;
    matcher->expecation_to_match->size = size;
    matcher->empty = 0;

=======
    matcher->current_expectation_index--;
    // printf("Cleared expecations. Index = %i \n", matcher->current_expectation_index);
>>>>>>> Stashed changes
}

Register *matchExpectedRegister()
{
    
    // If no expectatins, return GPR
    if (matcher->empty)
    {
       return getGPR(matcher->expecation_to_match->size); 
    }
   
    matcher->empty = 1;

    switch (matcher->expecation_to_match->registerType)
    {
        case ANY:
        case GPR: return getGPR(matcher->expecation_to_match->size);
        case FPR: return getFPR(matcher->expecation_to_match->size);
        default:
        {
            fprintf(stderr, "Unknown register type to match. Got : %i \n", matcher->expecation_to_match->registerType);
            exit(1);
        }
    }
}

void populateGPR(int index, char *name)
{
    // 64-bit register
    gpr->registers[index]->name = strdup(name);
    gpr->registers[index]->is_being_used = 0;
    gpr->registers[index]->size = 8;
    gpr->registers[index]->parentRegister = NULL;
    gpr->registers[index]->parent64Register = NULL;
    
    // 32-bit subregister (eax, ebx, etc.)
    char *eax_buffer = malloc(4);
    sprintf(eax_buffer, "e%cx", name[1]);  // "eax", "ebx", etc.
    gpr->registers[index]->subRegisters[0]->name = eax_buffer;
    gpr->registers[index]->subRegisters[0]->size = 4;
    gpr->registers[index]->subRegisters[0]->is_being_used = 0;
    gpr->registers[index]->subRegisters[0]->parentRegister = gpr->registers[index];
    gpr->registers[index]->subRegisters[0]->parent64Register = gpr->registers[index];

    // 16-bit subregister (ax, bx, etc.)
    char *ax_buffer = malloc(3);
    sprintf(ax_buffer, "%cx", name[1]);    // "ax", "bx", etc.
    gpr->registers[index]->subRegisters[1]->name = ax_buffer;
    gpr->registers[index]->subRegisters[1]->size = 2;
    gpr->registers[index]->subRegisters[1]->is_being_used = 0;
    gpr->registers[index]->subRegisters[1]->parentRegister = gpr->registers[index]->subRegisters[0];
    gpr->registers[index]->subRegisters[1]->parent64Register = gpr->registers[index];
    

    // 8-bit subregister (al, bl, etc.)
    char *al_buffer = malloc(3);
    sprintf(al_buffer, "%cl", name[1]);    // "al", "bl", etc.
    gpr->registers[index]->subRegisters[2]->name = al_buffer;
    gpr->registers[index]->subRegisters[2]->size = 1;
    gpr->registers[index]->subRegisters[2]->is_being_used = 0;
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

    populateGPR(0, "rax");
    populateGPR(1, "rbx");
    populateGPR(2, "rcx");
    populateGPR(3, "rdx");
}

Register *populateFPR(int index, char *name)
{

    // 64-bit register
    fpr->registers[index]->name = strdup(name);
    fpr->registers[index]->is_being_used = 0;
    fpr->registers[index]->size = 8;
    fpr->registers[index]->parentRegister = NULL;
    fpr->registers[index]->parent64Register = NULL;

    return fpr->registers[index];
}

void populateFPRSubRegisters(Register *parent64, char *name1, char *name2, char *name3)
{
    // 32-bit subregister (eax, ebx, etc.)
    parent64->subRegisters[0]->name = strdup(name1);
    parent64->subRegisters[0]->size = 4;
    parent64->subRegisters[0]->is_being_used = 0;
    parent64->subRegisters[0]->parentRegister = parent64;
    parent64->subRegisters[0]->parent64Register = parent64;

    // 16-bit subregister (ax, bx, etc.)
    parent64->subRegisters[1]->name = strdup(name2);
    parent64->subRegisters[1]->size = 2;
    parent64->subRegisters[1]->is_being_used = 0;
    parent64->subRegisters[1]->parentRegister = parent64->subRegisters[0];
    parent64->subRegisters[1]->parent64Register = parent64;
    

    // 8-bit subregister (al, bl, etc.)
    parent64->subRegisters[2]->name = strdup(name3);
    parent64->subRegisters[2]->size = 1;
    parent64->subRegisters[2]->is_being_used = 0;
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
    Register *rdi = populateFPR(0, "rsi");
    populateFPRSubRegisters(rdi, "edi", "di", "dil");
    
    // 2nd argument 
    Register *rsi = populateFPR(1, "rsi");
    populateFPRSubRegisters(rsi, "esi", "si", "sil");
    
    // 3rd argument 
    Register *rdx = populateFPR(2, "rdx");
    populateFPRSubRegisters(rdx, "edx", "dx", "dl");
    
    // 4th argument 
    Register *rcx = populateFPR(3, "rcx");
    populateFPRSubRegisters(rcx, "ecx", "cx", "cl");

    // 5th argument 
    Register *r8 = populateFPR(4, "r8");
    populateFPRSubRegisters(r8, "r8d", "r8w", "r8b");

    // 6th argument 
    Register *r9 = populateFPR(5, "r9");
    populateFPRSubRegisters(r9, "r9d", "r9w", "r9b");

}


Register *getTruncatedRegister(Register *reg, int size)
{
    if (size == reg->size) { return reg; }
    
    if (size > reg->size)
    {
        Register *current = reg;
        while (current != NULL && current->size != size)
        {
            current = current->parentRegister;
        }

        return current;
    }

    switch (size) 
    {
        case 4: return reg->subRegisters[0];
        case 2: return reg->subRegisters[1];
        case 1: return reg->subRegisters[2];
        default: 
        {
            fprintf(stderr, "Can't truncate to size: %i. No register/subregister matches that size. \n", size);
            exit(1);
        }
    }

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


// If we use a register / subregister, every subregister/parent is also busy.
void busyRegister(Register *reg)
{
    Register *parent = reg->parent64Register != NULL? reg->parent64Register : reg;

    parent->is_being_used = 1;
    for (int i = 0; i < 3; i++)
    {
        if (parent->subRegisters[i] != NULL)
        {
            parent->subRegisters[i]->is_being_used = 1;
        }
    }
}

void freeRegister(Register *reg)
{

    Register *parent = reg->parent64Register != NULL? reg->parent64Register : reg;

    parent->is_being_used = 0;
    for (int i = 0; i < 3; i++)
    {
        if (parent->subRegisters[i] != NULL)
        {
            parent->subRegisters[i]->is_being_used = 0;
        }
    }
}

void clearRegisters(RegisterTable *table)
{
    for (int i = 0; i < table->num_of_registers; i++)
    {
        freeRegister(table->registers[i]);
    }
}

Register *getDirectTemporaryRegisterForSize(int size)
{
    if (get_current_bt_context() == CTX_FUNC_CALL) { return getFPR(size); }

    return getGPR(size); 
}


Register *getFPR(int size)
{

    //printf("============================= \n");

    for (int i = 0; i < fpr->num_of_registers; i++)
    {
        //printf("Looking for FPR %s, checking if its free: %i \n", fpr->registers[i]->name, fpr->registers[i]->is_being_used);
        // Get a completely free register 
        if (fpr->registers[i]->is_being_used == 0) 
        {
            //printf("FPR register: %s is free, returning that one \n", fpr->registers[i]->name);
            switch (size)
            {
                case 8: { busyRegister(fpr->registers[i]); return fpr->registers[i]; }
                case 4: { busyRegister(fpr->registers[i]->subRegisters[0]); return fpr->registers[i]->subRegisters[0]; }
                case 2: { busyRegister(fpr->registers[i]->subRegisters[1]); return fpr->registers[i]->subRegisters[1]; }
                case 1: { busyRegister(fpr->registers[i]->subRegisters[2]); return fpr->registers[i]->subRegisters[2]; }
                default:
                {
                    fprintf(stderr, "Cannot get a register for size: %i\n", size);
                    exit(1);
                }
            }
        
        }
    }

    fprintf(stderr, "Can't get FPR temporary register for size: %i. All are being used\n", size);
    exit(1);
}


Register *getGPR(int size)
{
    
    for (int i = 0; i < gpr->num_of_registers; i++)
    {
        // Get a completely free register 
        if (gpr->registers[i]->is_being_used == 0) 
        {

            switch (size)
            {
                case 8: { busyRegister(gpr->registers[i]); return gpr->registers[i]; }
                case 4: { busyRegister(gpr->registers[i]->subRegisters[0]); return gpr->registers[i]->subRegisters[0]; }
                case 2: { busyRegister(gpr->registers[i]->subRegisters[1]); return gpr->registers[i]->subRegisters[1]; }
                case 1: { busyRegister(gpr->registers[i]->subRegisters[2]); return gpr->registers[i]->subRegisters[2]; }
                default:
                {
                    fprintf(stderr, "Cannot get a register for size: %i\n", size);
                    exit(1);
                }
            }

        }
    }

    fprintf(stderr, "Can't get GPR temporary register for size: %i. All are being used\n", size);
    exit(1);
}
void increment_bt_ctx_block_id(ContextType ctx_type)
{
    switch (ctx_type) 
    {
        case CTX_IF: bt_context.if_block_id++; return;
        case CTX_ELSE: bt_context.else_block_id++; return;
        case CTX_FOR: bt_context.for_block_id++; return;
        case CTX_FOREACH: bt_context.foreach_block_id++; return;
        case CTX_WHILE: bt_context.while_block_id++; return;
        case CTX_FUNCTION: bt_context.function_block_id++; return;
        case CTX_NONE:
        {
            fprintf(stderr, "Cannot increment NONE_block_id as there is only one global scope. \n");
            exit(1);
        }
    }
}



void push_bt_context(ContextType ctx_type)
{
    if (bt_context.current_scope + 1 >= MAX_NESTING)
    {
        fprintf(stderr, "Cannot push another context (%s), max nesting is %i \n", 
                ctxToString(ctx_type) ,MAX_NESTING);
    }
    
    increment_bt_ctx_block_id(ctx_type);
    bt_context.current_scope++;
    bt_context.ctx_type_stack[bt_context.current_scope] = ctx_type;
}

void pop_bt_context()
{
    if (bt_context.current_scope - 1 < 0)
    {
        fprintf(stderr, "Can't pop context if context is CTX_NONE. \n");
        exit(1);
    }
    bt_context.current_scope--;
}



ContextType get_current_bt_context()
{
    return bt_context.ctx_type_stack[bt_context.current_scope];
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
    if (strcmp(str, "&&") == 0) {return AND;}
    if (strcmp(str, "||") == 0) {return OR;}

    return UNKNOWN;
}

int strToUnaryOPCode(char *str)
{
    if (strcmp(str, "-") == 0) { return UNARY_MINUS; }
    if (strcmp(str, "!") == 0) { return UNARY_NOT; }
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

    fprintf(stderr, "No word matches the given size: %i \n", size);
    exit(1);
}


int compute_binaryOperationDepth(ASTNode *node, int depth)
{
<<<<<<< Updated upstream
    if ( node == NULL || node->node_type == NODE_NUMBER || node->node_type == NODE_NULL) 
    {
        return depth;
    }

    if ( node->node_type == NODE_UNARY_OP)
    {
        return depth + 1;
    }
    
    BinaryOPNode *binary_op_node = (BinaryOPNode*)node;

    int sub_left = compute_binaryOperationDepth(binary_op_node->left, depth + 1);
    int sub_right = compute_binaryOperationDepth(binary_op_node->right, depth + 1);
=======
    printf("Visiting node type %d at depth %d\n", node->node_type, depth);
    switch (node->node_type) 
    {
        case NODE_NULL:
        case NODE_NUMBER:
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
>>>>>>> Stashed changes
}


char *short_for_type(char *type)
{
<<<<<<< Updated upstream
    if (strcmp(type, "uint8") == 0) { return "u8"; }
    if (strcmp(type, "uint16") == 0) { return "u16"; }
    if (strcmp(type, "uint32") == 0) { return "u32"; }
    if (strcmp(type, "int8") == 0) { return "i8"; }
    if (strcmp(type, "int16") == 0) { return "i16"; }
    if (strcmp(type, "int32") == 0) { return "i32"; }
    if (strcmp(type, "char") == 0) { return "c"; }
    if (strcmp(type, "bool") == 0) { return "b"; }
    if (strcmp(type, "str") == 0) { return "s"; }
=======
    /* 1. Make a writable copy and strip all whitespace */
    char cleaned[256];                          /* adjust size if you expect very long names */
    remove_whitespace_copy(cleaned, type, sizeof(cleaned));
>>>>>>> Stashed changes

    /* 2. Count pointer levels and move pointer to the base type */
    const char *base = cleaned;
    int ptr_level = count_ptr_levels(&base);

    /* 3. Shorten the base type */
    const char *short_base;
    if      (strcmp(base, "uint8")  == 0) short_base = "u8";
    else if (strcmp(base, "uint16") == 0) short_base = "u16";
    else if (strcmp(base, "uint32") == 0) short_base = "u32";
    else if (strcmp(base, "int8")   == 0) short_base = "i8";
    else if (strcmp(base, "int16")  == 0) short_base = "i16";
    else if (strcmp(base, "int32")  == 0) short_base = "i32";
    else if (strcmp(base, "char")   == 0) short_base = "c";
    else if (strcmp(base, "bool")   == 0) short_base = "b";
    else if (strcmp(base, "str")    == 0) short_base = "s";
    else short_base = base;                     /* user-defined type – keep as-is */

    /* 4. Build the result: p-repeated + short_base */
    static char result[128];                    /* big enough for many p's + name */
    int pos = 0;

    for (int i = 0; i < ptr_level; i++)
        result[pos++] = 'p';

    strcpy(result + pos, short_base);
    return result;
}

char *write_loop_signature(FILE *asm_file, ASTNode *program)
{
    char *label = malloc(100 * sizeof(char));
    switch (program->node_type)
    {
        case NODE_WHILE:
        {
            emit_comment(asm_file, "While loop");
            fprintf(asm_file, ".while_loop_%i:\n", bt_context.while_block_id);
            sprintf(label, ".while_loop_%i", bt_context.while_block_id);
            break;
        }

        case NODE_FOR:
        {
            emit_comment(asm_file, " loop");
            fprintf(asm_file,".for_loop_%i:\n", bt_context.for_block_id);
            sprintf(label, ".for_loop_%i", bt_context.for_block_id);
            break;
        }
    }

    return label;
}


<<<<<<< Updated upstream

void write_function_signature(FILE *asm_file, ASTNode *program)
=======
char *write_if_signature(FILE *asm_file)
{
    char *label = malloc(100 * sizeof(char));
    
    emit_comment(asm_file, "If statement");
    fprintf(asm_file, ".if_stmt_%i:\n", bt_context.if_block_id);
    sprintf(label, ".if_stmt_%i", bt_context.if_block_id);

    return label;

}


char *getRetLabelFromFunction(Function *f)
{
    char **param_types = get_paramTypes_from_Params(f->params, f->param_count);
    char *ret_label = malloc(sizeof(char) * 200);
    memset(ret_label, 0, 200); // Zero the buffer


    sprintf(ret_label, ".ret_from_%s", f->identifier);
   

    for (int i = 0; i < f->param_count; i++)
    {
        char buffer[32] = {0};
        snprintf(buffer, sizeof(buffer),"_%s", short_for_type(param_types[i]));
        strcat(ret_label, buffer);
    }

    
    ret_label = realloc(ret_label, strlen(ret_label) + 1);
    return ret_label;

}

char *write_function_signature(FILE *asm_file, ASTNode *program)   
>>>>>>> Stashed changes
{

    char **param_types;

    switch(program->node_type)
    {
        case NODE_EXTERN_FUNC_DEF:
        {
            fprintf(asm_file, "extern _%s", program->extern_func_def_node.func_name);

            param_types = get_paramTypes_from_Params(program->extern_func_def_node.params, program->extern_func_def_node.params_count);
            for (int i = 0; i < program->extern_func_def_node.params_count; i++)
            {
                fprintf(asm_file, "_%s", short_for_type(param_types[i])); 
            }
            fprintf(asm_file, "\n");

            break;
        }

        case NODE_STDALONE_FUNC_CALL:
        {
            fprintf(asm_file, "    call _%s", program->stdalone_func_call_node.identifier);

            param_types = program->stdalone_func_call_node.params_type;
            for (int i = 0; i < program->stdalone_func_call_node.params_count; i++)
            {
                fprintf(asm_file, "_%s", short_for_type(param_types[i])); 
            }
            fprintf(asm_file, "\n");
            break;
        }

        case NODE_FUNC_CALL:
        {
            fprintf(asm_file, "    call _%s", program->funccall_node.identifier);
            param_types = program->funccall_node.params_type;
            for (int i = 0; i < program->funccall_node.params_count; i++)
            {
                fprintf(asm_file, "_%s", short_for_type(param_types[i]));
            }
            fprintf(asm_file, "\n");
            break;
        }

        case NODE_FUNC_DEF:
        {
            fprintf(asm_file, "_%s", program->funcdef_node.func_name);
            param_types = get_paramTypes_from_Params(program->funcdef_node.params, program->funcdef_node.params_count);
            for (int i = 0; i < program->funcdef_node.params_count; i++)
            {
                fprintf(asm_file, "_%s", short_for_type(param_types[i]));
            }
            fprintf(asm_file, ":\n");
            break;
        }

        default:
        {
            fprintf(stderr, "Cannot write function signature in current node: %s \n", astTypeToStr(program));
            exit(1);
        }
    }
 
}


Operand buildData(FILE *asm_file, ASTNode *program, SymbolTable *current_st)
{
    switch(program->node_type)
    {
        case NODE_BLOCK:
        {
            for (int i = 0; i < program->block_node.statement_count; i++)
            {
               buildData(asm_file, program->block_node.statements[i], current_st); 
            }
            
            Operand none = {.type = NONE};
            return none;
        }

        case NODE_USE_DIRECTIVE:
        {
            buildData(asm_file, program->use_node.program, current_st);
            Operand none = {.type = NONE};
            return none;
        }

        case NODE_ASSIGNMENT:
        {
            int size = size_of_type(program->assignment_node.type);
            
            int64_t default_value = 0;
            if (program->assignment_node.expression->node_type == NODE_NUMBER)
            {
                default_value = program->assignment_node.expression->number_node.number_value;
            }
            
            char *define_enough_bytes = defineWordForSize(size);
            fprintf(asm_file, "    %s %s %"PRId64"\n", program->assignment_node.identifier, define_enough_bytes, default_value);
            break;
        }

        case NODE_REASSIGNMENT:
        {
            // Can only be at scope 0, these are globals 
            Symbol *s = lookup_identifier(current_st, 0, program->reassignment_node.identifier);
            int size = size_of_type(s->type);
            
            int64_t default_value = 0;
            if (program->reassignment_node.expression->node_type == NODE_NUMBER)
            {
                default_value = program->reassignment_node.expression->number_node.number_value;
            }
            
            

            char *define_enough_bytes = defineWordForSize(size);
            fprintf(asm_file, "    %s %s %"PRId64"\n", s->identifier, define_enough_bytes, default_value);
            break;
        }

        case NODE_DECLARATION:
        {
            int size = size_of_type(program->declaration_node.type);
             
            char *define_enough_bytes = defineWordForSize(size);
            fprintf(asm_file, "    %s %s 0\n", program->declaration_node.identifier, define_enough_bytes);
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
            
            Operand none = {.type = NONE};
            return none;
        }

        case NODE_USE_DIRECTIVE:
        {
            buildExtern(asm_file, program->use_node.program, current_ft);
            Operand none = {.type = NONE};
            return none;
        }

        case NODE_EXTERN_FUNC_DEF:
        {

            write_function_signature(asm_file, program);
            Operand none = {.type =  NONE};
            return none;
        }

        default:
            break;


    }
}

Operand buildStart(FILE *asm_file, ASTNode *program, SymbolTable *current_st, FunctionTable *current_ft)
{
<<<<<<< Updated upstream
    // printf("Building for node type: %s \n", astTypeToStr(program));
=======
    printf("Building for node: %s \n", astTypeToStr(program));
    if (program == NULL)
    {
        fprintf(stderr, "Trying to build for a NULL node \n");
        exit(1);
    }
>>>>>>> Stashed changes
    switch (program->node_type) 
    {   
        case NODE_BLOCK:
        {
            for (int i = 0; i < program->block_node.statement_count; i++)
            {
               buildStart(asm_file, program->block_node.statements[i], current_st, current_ft); 
            }
            
            Operand none = {.type = NONE};
            return none;
        }

        case NODE_USE_DIRECTIVE:
        {
            buildStart(asm_file, program->use_node.program, current_st, current_ft);
            Operand none = {.type = NONE};
            return none;
        }

        case NODE_CAST:
        {
            Operand casting_expr = buildStart(asm_file, program->cast_node.expr, current_st, current_ft); 
            return casting_expr;
        }

        case NODE_WHILE:
        {
            push_bt_context(CTX_WHILE);
            char *label = write_loop_signature(asm_file, program);
            char *end = malloc(120 * sizeof(char));
            sprintf(end, ".end_%s", label);

            // Condition check
            emit_comment(asm_file, "Condition check");
            Operand condition = buildStart(asm_file, program->while_node.condition_expr, current_st, current_ft);
            freeRegister(condition.linkedToRegister);
            // printf("Condition operand: Linked to: %s\n", condition.linkedToRegister);
            fprintf(asm_file, "    cmp %s, 0\n", condition.linkedToRegister->name);
            fprintf(asm_file, "    je %s\n",end);


            emit_comment(asm_file, "Body");
            buildStart(asm_file, program->while_node.body, current_st, current_ft);


            emit_comment(asm_file, "Back to conditon check");
            fprintf(asm_file, "    jmp %s\n\n", label);
            fprintf(asm_file, "%s:\n", end);

            pop_bt_context();
            break;
        }

        case NODE_FOR:
        {
            push_bt_context(CTX_FOR);

            // Initial assignment 
            emit_comment(asm_file, "Initial assignment of for loop");
            Operand assignment = buildStart(asm_file, program->for_node.assignment_expr, current_st, current_ft);

            char *label = write_loop_signature(asm_file, program);
            char *end = malloc(120 * sizeof(char));
            sprintf(end, ".end_%s", label);
            
            // Condition check 
            pushWorkingContext(COMPUTING_RVALUE);
            emit_comment(asm_file, "Condition check");
            Operand condition = buildStart(asm_file, program->for_node.condition_expr, current_st, current_ft);
            freeRegister(condition.linkedToRegister);
            fprintf(asm_file, "    cmp %s, 0\n", condition.linkedToRegister->name);
            fprintf(asm_file, "    je %s\n", end);
            popWorkingContext();

            // Body
            emit_comment(asm_file, "Body");
            buildStart(asm_file, program->for_node.body, current_st, current_ft);
            // Reassignment 
            emit_comment(asm_file, "Reassignment for index");
            buildStart(asm_file, program->for_node.reassignment_expr, current_st, current_ft);
            // Back to condition check 
            emit_comment(asm_file, "Back to condition check");
            fprintf(asm_file,"    jmp %s\n\n", label);
            fprintf(asm_file,"%s:\n", end);

            pop_bt_context();
            break;
        }


        case NODE_FUNC_DEF:
        {
            push_bt_context(CTX_FUNCTION);
            

            write_function_signature(asm_file, program);
            // Prelude
            // Save RBP 
            fprintf(asm_file,"    push rbp\n");
            // Use RBP to reference the stack frame 
            fprintf(asm_file,"    mov rbp, rsp\n");

            // Get the function  
            char **param_types = get_paramTypes_from_Params(program->funcdef_node.params, program->funcdef_node.params_count);

            Function *f = lookup_function(current_ft, bt_context.current_scope, 
                                          program->funcdef_node.func_name, param_types, program->funcdef_node.params_count);
            // Reserve stack for variables. There's already 8 when doing push rbp. Align to 16-byte  
            //
            // align_size + 8 = 16 * m, where m natural >= 1. align_size >= weight 
            // best case scenario: weight + 8 is a multiple of 16 => no over allocation 
            // worst case sceneario: weight + 8 = 16*m - 1 for a given m, then there is 15 bytes of over-allocation 

            int total = (f->local_symbols->weight + 8 + 15) & ~15; // Round up weight + 8 to multiple of 16
            int align_size = total - 8; // Subtract push rbp
            if (align_size < 8) align_size = 8;
            
<<<<<<< Updated upstream
            fprintf(asm_file,"    ;Align to 16 bytes\n");
            fprintf(asm_file,"    sub rsp, %i\n", align_size);
=======
            updateCFF(f);
                      
            emit_comment(asm_file, "Align to 16 bytes");
            fprintf(asm_file,"    sub rsp, %i\n", getStackBasePtr());
>>>>>>> Stashed changes


            // Load the parameters 
            emit_comment(asm_file, "Loading parameters");
            int params_to_it_until = min(6, f->param_count);
            for (int i = 0; i < params_to_it_until; i++)
            {
                Symbol *param = get_ParamAtIndex(f->local_symbols, i);
                char *word = getWordForSize(param->size);
                // Expectate a FPR register with the size of the parameter 
<<<<<<< Updated upstream
                expectRegister(FPR, size_of_type(param->type));
                // Match it 
                fprintf(asm_file, "    mov [rbp - %i], %s \n", 1 + param->offset, matchExpectedRegister()->name);
            }

            
            // Clear the FPR 
            clearRegisters(fpr);

=======
                expectRegister(FPR, param->size);
                // Match it 
                fprintf(asm_file, "    mov %s [rbp - %i], %s \n", word,
                        getStackBasePtr() - param->offset, matchExpectedRegister()->name);
                clearExpectation();
            }

            // If we have more than 6 params, calculate their stack position and place them
            // at the correct current stack frame position
            int is_param_count_odd = f->param_count % 2 != 0 ? 1 : 0;
            if (f->param_count > 6)
            {
                for (int i = f->param_count - 1; i >= 6; i--)
                {
                    Symbol *p = get_ParamAtIndex(f->local_symbols, i);
                    expectRegister(GPR, p->size);
                    Register *temp = matchExpectedRegister();
                    int argument_offset = 8 + 8 * is_param_count_odd + 8 * (f->param_count - i);
                    fprintf(asm_file, "    mov %s, [rbp + %i]     ; %i th argument\n", temp->name, argument_offset, i + 1); // rbp + (padding and weight) + call + 8 * (N - 6)
                    fprintf(asm_file, "    mov %s [rbp - %i], %s \n", getWordForSize(temp->size),
                            getStackBasePtr() - p->offset, temp->name);
                    freeRegister(temp);
                    clearExpectation();
                }
            }

             
>>>>>>> Stashed changes
            // Instructions 
            buildStart(asm_file, program->funcdef_node.body, f->local_symbols, f->local_functions ); 

            // Unreserve the stack 
<<<<<<< Updated upstream
            fprintf(asm_file,"    ;Restore the stack pointer\n");
            fprintf(asm_file,"    add rsp, %i\n", align_size);
=======
            emit_comment(asm_file, "Restore the stack pointer and return");
            fprintf(asm_file,".ret_from_%s:\n", func_signature);



            fprintf(asm_file,"    add rsp, %i\n", getStackBasePtr());

>>>>>>> Stashed changes
            // Restore RBP 
            fprintf(asm_file,"    pop rbp\n");
            // Return (If 7 or more arguments used, gotta use ret <N> where N = # args in stack x bytes each)
            fprintf(asm_file,"    ret\n\n");

            pop_bt_context();
            break;
        }
        
        case NODE_STDALONE_FUNC_CALL:
        case NODE_FUNC_CALL:
        {
            // Push register context 
            push_bt_context(CTX_FUNC_CALL);
            // Get the function we are calling 
            Function *f = lookup_function(current_ft, bt_context.current_scope, 
                                          program->funccall_node.identifier, program->funccall_node.params_type, program->funccall_node.params_count);
            // Clear the registers 
            clearRegisters(gpr);
            clearRegisters(fpr);


            emit_comment(asm_file, "Preparing to call function");

<<<<<<< Updated upstream

            // Build the expressions for the parameters 
            for (int i = 0; i < program->funccall_node.params_count; i++)
            {
                expectRegister(FPR, size_of_type(program->funccall_node.params_type[i]));
                buildStart(asm_file, program->funccall_node.params_expr[i], current_st, current_ft);
            }
            

            write_function_signature(asm_file, program);
            
            pop_bt_context();
            //Return value is always in RAX 
            Operand return_value = {.linkedToRegister = gpr->registers[0] };
=======
            // Save caller-saved registers, push
            // Check if any is being used
            // Keep track of the number of push reg to ensure rsp alignment
            int count;
            Register **used_registers = getBusyGPRs(&count);
            int pushed_registers = 0;

            int raxPushed = 0;

            if (used_registers != NULL)
            {
                emit_comment(asm_file, "Saving caller saved registers");
                for (int i = 0; i < count; i++)
                {
                    // Dont push rax. 22/11? why not?
                    // check for used_registers[i]->is_caller_saved
                    // if (used_registers[i] != gpr->registers[0])
                    // {
                    //     fprintf(asm_file, "    push %s \n", used_registers[i]->name);
                    //     pushed_registers++;
                    // }
                    
                    // If we have pushed rax
                    if (used_registers[i] == gpr->registers[0]) { raxPushed = 1; }
                    fprintf(asm_file, "    push %s \n", used_registers[i]->name);
                    pushed_registers++;
                }
            }



            
            Operand par;
            int number_of_arg_pushes = 0;

            // Build the expressions for the parameters 
            for (int i = 0; i < program->funccall_node.params_count; i++)
            {
                 // Push to the stack if >6 parameters
                if (i >= 6)
                {

                    par = with_rvalue_context(asm_file, program->funccall_node.params_expr[i], GPR,
                                    size_of_type(program->funccall_node.params_type[i]), current_st, current_ft);

                    fprintf(asm_file, "    push %s    ; %i argument \n", par.linkedToRegister->parent64Register->name, i + 1);
                    number_of_arg_pushes++;

                    freeRegister(par.linkedToRegister);
                

                    continue;
                }

                par = with_rvalue_context(asm_file, program->funccall_node.params_expr[i], FPR, 
                                    size_of_type(program->funccall_node.params_type[i]), current_st, current_ft);
            

                
                //Ensure the parameters are stored in the correct FPR registers, otherwise move it in
                if (par.linkedToRegister->type != FPR)
                {
                    Register *fpr_reg = getFPR(par.linkedToRegister->size);
                    fprintf(asm_file, "    ;Changing to FPR, par was stored in GPR: %s\n", par.linkedToRegister->name);
                    fprintf(asm_file, "    mov %s, %s\n", fpr_reg->name, par.linkedToRegister->name);
                }
            }
            


            // Ensure 16 byte alignment 
            if ((number_of_arg_pushes + pushed_registers) % 2 != 0) { fprintf(asm_file, "    sub rsp, 8     ;Ensure rsp 16-byte alignment\n"); }
            


            

            // call func_label
            write_function_signature(asm_file, program);
            
            



        
            exit_node(program);

            
            Operand return_value;
            return_value.type = NONE;
            
            if (strcmp(f->rt_type, "void") != 0)
            {
                //Return value is always in RAX. If RAX wasnt pushed, its fine, result is here and wont overwrite
                if (!raxPushed)
                {
                    return_value.linkedToRegister = getTruncatedRegister(gpr->registers[0], size_of_type(f->rt_type));
                }
                
                // If RAX was pushed, when we pop it, we may overwrite. Move the returned value somewhere else
                else 
                {
                    Register *r = getDirectTemporaryRegisterForSize(size_of_type(f->rt_type));
                    fprintf(asm_file, "    mov %s, %s \n", r->name, getFamilyRegWithSize(gpr->registers[0], size_of_type(f->rt_type))->name);
                    return_value.linkedToRegister = r;
                    busyRegister(r);
                }
                busyRegister(return_value.linkedToRegister);
                emit_comment(asm_file, "Ended func call node");
            }


            // Make up for the potential sub rsp, 8 from before 
            if ((number_of_arg_pushes + pushed_registers) % 2 != 0) { fprintf(asm_file, "    add rsp, 8     ;Ensure rsp 16-byte alignment\n"); }


            // Retrieve them back, pop 
            if (used_registers != NULL)
            {
                emit_comment(asm_file, "Retrieving caller-saved registers");
                for (int i = count - 1; i >= 0; i--)
                {
                    // Dont pop rax. 22/11: why not?
                    // check for used_registers[i]->is_caller_saved

                    // if (used_registers[i] != gpr->registers[0])
                    // {
                    //     fprintf(asm_file, "    pop %s \n", used_registers[i]->name);
                    // }

                    fprintf(asm_file, "    pop %s \n", used_registers[i]->name);
                }
            }
            

            // Make up for all the pushed arguments


            
            emit_comment(asm_file, "Ended func call ndoe");

            
>>>>>>> Stashed changes
            return return_value;
        }

        case NODE_RETURN:
        {
<<<<<<< Updated upstream
            fprintf(asm_file, "    ; Return node\n");
            // Gotta put the return value in RAX 
            Operand return_value = buildStart(asm_file, program->return_node.return_expr, current_st, current_ft);
            return return_value;
=======
            enter_node(program);
            emit_comment(asm_file, "Return node");
            Operand ret;

            char *ret_label = getRetLabelFromFunction(getCurrentFunction());
            if (program->return_node.return_expr == NULL)
            {
                fprintf(asm_file, "    jmp %s\n", ret_label);
                ret.type = NONE;
                exit_node(program);
                return ret;
            }
            
            // Return size must match size declared in return type
            expectRegister(GPR, size_of_type(getCurrentFunction()->rt_type));
            ret = buildStart(asm_file, program->return_node.return_expr, current_st, current_ft);
            clearExpectation();
                
            // Ensure return is in RAX 
            if (! ((ret.linkedToRegister->parent64Register == NULL && strcmp(ret.linkedToRegister->name, "rax") == 0) 
                || (ret.linkedToRegister->parent64Register != NULL && strcmp(ret.linkedToRegister->parent64Register->name, "rax") == 0)))
            {
                emit_comment(asm_file, "Ensure return is in rax");
                fprintf(asm_file, "    mov %s, %s\n", getSubRegisterForSize(gpr, "rax", ret.linkedToRegister->size)->name , ret.linkedToRegister->name);
                freeRegister(ret.linkedToRegister);
            }


            fprintf(asm_file, "    jmp %s\n", ret_label);
            exit_node(program);
            return ret;
        }
        
        
        case NODE_ARRAY_INIT:
        {
            enter_node(program);

            emit_comment(asm_file, "Array init");
            // Get the expected register 
            Symbol *arr = lookup_identifier(current_st, bt_context.current_scope, program->array_init_node.arr_name);
            char *element_type = get_arr_element_type(arr->type);
            int size_of_every_element = size_of_type(element_type); 

            Register *first_element = NULL;
            

            // rep movsb


            //lea rdi, [dest]
            //lea rsi, [src]
            //mov rcx, size
            //rep movsb
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

                    if (i == 0)  { first_element = result.linkedToRegister; }

                    freeRegister(result.linkedToRegister);
                }
            }

            // normal mov
            else 
            {
                for (int i = 0; i < program->array_init_node.size; i++)
                {
                    // Expect a similar one for every entry 
                    expectRegister(ANY, size_of_every_element);
                    Operand result = buildStart(asm_file, program->array_init_node.elements[i], current_st, current_ft);

                    fprintf(asm_file, "    mov %s [rbp - %i], %s \n", getWordForSize(size_of_every_element),
                            getStackBasePtr() - (i * size_of_every_element + arr->offset), getTruncatedRegister(result.linkedToRegister, size_of_every_element)->name);

                    if (i == 0) 
                    {
                        first_element = result.linkedToRegister;
                    }
                    //Free that register, can be immedietaly used after 
                    freeRegister(result.linkedToRegister);
                    clearExpectation();
                }
            }

            
            emit_comment(asm_file, "Array init end");
            
            exit_node(program);
            Operand result = {.linkedToRegister = first_element};
            return result;
        }

        case NODE_SUBSCRIPT: 
        {

            enter_node(program);

            emit_comment(asm_file, "Array subscript");

            // Compute the base address
            emit_comment(asm_file, "Computing the base address");
            pushWorkingContext(COMPUTING_LVALUE);
            expectRegister(GPR, 8);
            Operand base = buildStart(asm_file,program->subscript_node.base, current_st, current_ft);
            clearExpectation();
            popWorkingContext();



            // Compute the index 
            emit_comment(asm_file, "Computing the index");
            pushWorkingContext(COMPUTING_RVALUE);
            // Compute the index on a 8 byte register
            expectRegister(GPR, 8);
            Operand index = buildStart(asm_file, program->subscript_node.index, current_st, current_ft);
            popWorkingContext();
            clearExpectation();


            // clearExpectation();
            // imul r8,r8 is not supported.
            emit_comment(asm_file, "Computing the position of the element to retrieve");


            emit_mul_imm(asm_file, index, program->subscript_node.element_size, 8);

            if (index.linkedToRegister->parent64Register == NULL) { printf("WTF \n"); }
            //fprintf(asm_file, "    imul %s, %i \n", index.linkedToRegister->name, size_of_base);
            
            // If the index is placed on an 8 byte register then parent64 is null

            if (index.linkedToRegister->parent64Register != NULL)
            {
                fprintf(asm_file, "    movzx %s, %s \n", index.linkedToRegister->parent64Register->name, index.linkedToRegister->name); 
                fprintf(asm_file, "    add %s, %s \n", base.linkedToRegister->name, index.linkedToRegister->parent64Register->name);
            }

            else 
            {
                fprintf(asm_file, "    add %s, %s \n", base.linkedToRegister->name, index.linkedToRegister->name);
            }




            freeRegister(index.linkedToRegister);
            Register *result = base.linkedToRegister;
            // If computing RVALUE, we want the value, not the address 
            if (getCurrentWorkingContext() == COMPUTING_RVALUE) 
            {
                result = matchExpectedRegisterOrDefault(GPR, 8);
                fprintf(asm_file, "    mov %s, [%s]\n", result->name, base.linkedToRegister->name);
                freeRegister(base.linkedToRegister);
            }

            emit_comment(asm_file, "Array subscript end");
            exit_node(program);


            Operand efective_address = {.linkedToRegister = result };
            return efective_address;
           
>>>>>>> Stashed changes
        }

        case NODE_FIELD_ACCESS:
        {
            enter_node(program);
            emit_comment(asm_file, "Field access");

            Operand base = with_lvalue_context(asm_file, program->field_access_node.base, current_st, current_ft);
            
            Object *o = lookup_object(gb_objectTable, program->field_access_node.type);

            Symbol *field = lookup_identifier(o->local_symbols, 1, program->field_access_node.field_name);

            fprintf(asm_file, "    add %s, %i \n", base.linkedToRegister->name, field->offset);   // Address of base.field in base.linkedtoRegister

            // If computing RVALUE we want the value, not the address 
            Register *result = base.linkedToRegister;
            if (getCurrentWorkingContext() == COMPUTING_RVALUE)
            {
                result = matchExpectedRegisterOrDefault(GPR, base.linkedToRegister->size );
                fprintf(asm_file, "    mov %s,[%s] \n", result->name, base.linkedToRegister->name);
                freeRegister(base.linkedToRegister);
            }

            emit_comment(asm_file, "Field access end");
            exit_node(program);

            Operand effective_address = {.linkedToRegister = result};
            return effective_address;
        }

        case NODE_PTR_FIELD_ACCESS:
        {
            enter_node(program);
            emit_comment(asm_file, "Ptr field access");


            Operand base = with_lvalue_context(asm_file, program->ptr_field_access_node.base, current_st, current_ft);
            fprintf(asm_file, "    mov %s, [%s] \n", base.linkedToRegister->name, base.linkedToRegister->name);


            char *pointed_type = get_pointed_type(program->ptr_field_access_node.type, 1);

            // Do something else here? We will see
            Object *o = lookup_object(gb_objectTable, pointed_type);

            Symbol *field = lookup_identifier(o->local_symbols, 1, program->ptr_field_access_node.field_name);

            fprintf(asm_file, "    add %s, %i \n", base.linkedToRegister->name, field->offset);

            // If computing RVALUE we want the value, not the address 
            Register *result = base.linkedToRegister;
            if (getCurrentWorkingContext() == COMPUTING_RVALUE)
            {
                result = matchExpectedRegisterOrDefault(GPR, base.linkedToRegister->size );
                fprintf(asm_file, "    mov %s,[%s] \n", result->name, base.linkedToRegister->name);
                freeRegister(base.linkedToRegister);
            }

            emit_comment(asm_file, "Ptr field access end");
            exit_node(program);


            Operand effective_address = {.linkedToRegister = result};
            return effective_address;
        }

        case NODE_ASSIGNMENT:
        {
            Operand none = {.type = NONE};
            // Early exit if the assignment is in no contex.
            // That means is global, which was done in the first pass
            if (get_current_bt_context() == CTX_NONE)
            {
                return none;
            }
            
<<<<<<< Updated upstream
            expectRegister(ANY, size_of_type(program->assignment_node.type));
            
            Operand result = buildStart(asm_file, program->assignment_node.expression, current_st, current_ft);
            Symbol *identifier = lookup_identifier(current_st, bt_context.current_scope, program->assignment_node.identifier);
            
            fprintf(asm_file, "    mov %s [rbp - %i], %s\n", getWordForSize(size_of_type(identifier->type)),
                    1 + identifier->offset, getTruncatedRegister(result.linkedToRegister, size_of_type(identifier->type))->name); 
            // Free result register 
            freeRegister(result.linkedToRegister);

=======
            int size = size_of_type(program->assignment_node.type);


            printf("Building assignment expression \n");
            Operand rvalue = with_rvalue_context(asm_file, program->assignment_node.expression, GPR, size, current_st, current_ft );
            printf("Got back \n");
            Symbol *identifier = lookup_identifier(current_st, bt_context.current_scope, program->assignment_node.identifier);

            

            // Avoid extra mov operations for array assignments. Only escalar assignments use this 
            if (program->assignment_node.expression->node_type != NODE_ARRAY_INIT)
            {
                fprintf(asm_file, "    mov %s [rbp - %i], %s\n", getWordForSize(size),
                    getStackBasePtr() - identifier->offset, getTruncatedRegister(rvalue.linkedToRegister, size)->name); 
            }
           

            exit_node(program);
>>>>>>> Stashed changes
            return none;
        }

        case NODE_REASSIGNMENT:
        {
            Operand none = {.type = NONE};

            if (get_current_bt_context() == CTX_NONE)
            {
                return none;
            }

            Operand lvalue;
            Operand rvalue;

            emit_comment(asm_file, "Reassignment");
            /*
            lea rdi, [dest]
            lea rsi, [src]
            mov rcx, size
            rep movsb
            */

 
            // 22/11: Later change to program->reassignment_node.size > 16 
            // use two registers to pass the value



            
<<<<<<< Updated upstream
            Symbol *s = lookup_identifier(current_st, bt_context.current_scope ,program->reassignment_node.identifier);
            expectRegister(ANY, size_of_type(s->type));
            Operand result = buildStart(asm_file, program->reassignment_node.expression, current_st, current_ft);
            
            // Handle smart reassignments like +=, -=, *=, /= 

            fprintf(asm_file, "    mov %s [rbp - %i], %s\n", getWordForSize(size_of_type(s->type)), 
                    1 + s->offset, getTruncatedRegister(result.linkedToRegister, size_of_type(s->type))->name);
=======
            if (program->reassignment_node.size > 8)
            {
                emit_comment(asm_file, "LValue");
                lvalue = with_lvalue_context(asm_file, program->reassignment_node.lvalue, current_st, current_ft);
                emit_comment(asm_file, "RValue (address)");
                rvalue = with_lvalue_context(asm_file, program->reassignment_node.expression, current_st, current_ft);
                emit_comment(asm_file, "Copying memory");
                fprintf(asm_file, "    lea rdi, [%s]\n", lvalue.linkedToRegister->name);
                fprintf(asm_file, "    lea rsi, [%s]\n", rvalue.linkedToRegister->name);
                fprintf(asm_file, "    mov rcx, %i \n", program->reassignment_node.size);
                fprintf(asm_file, "    rep movsb\n");
                

                // No need to handle smart reassignments here as they are not possible between structs
                exit_node(program);
                return none;
            }

            else 
            {
                emit_comment(asm_file, "LValue");
                lvalue = with_lvalue_context(asm_file, program->reassignment_node.lvalue, current_st, current_ft);
                emit_comment(asm_file, "Rvalue");
                rvalue = with_rvalue_context(asm_file, program->reassignment_node.expression, GPR, program->reassignment_node.size, current_st, current_ft);
            }



            // Handle smart reassignments like +=, -=, *=, /= 
            // For not pointer types
            if (!is_type_ptr(program->reassignment_node.type))
            {
                switch(program->reassignment_node.op)
                {
                    case TOKEN_ASSIGN:
                    {
                        fprintf(asm_file, "    mov %s [%s], %s \n", 
                            getWordForSize(rvalue.linkedToRegister->size), lvalue.linkedToRegister->name, rvalue.linkedToRegister->name);
                        break;
                    }

                    case TOKEN_ADD_ASSIGN:
                    {
                        fprintf(asm_file, "    add [%s], %s \n", lvalue.linkedToRegister->name, rvalue.linkedToRegister->name);
                        break;
                    }

                    case TOKEN_SUB_ASSIGN:
                    {
                        fprintf(asm_file, "    sub [%s], %s \n", lvalue.linkedToRegister->name, rvalue.linkedToRegister->name);
                        break;
                    }

                    case TOKEN_MUL_ASSIGN:
                    {
                        
                        break;
                    }

                    case TOKEN_MOD_ASSIGN:
                    {
                        break;
                    }

                    case TOKEN_DIV_ASSIGN:
                    {
                        break;
                    }

                }
            }


            // For reassignment with pointer types: p += n ---> p = p + n * sizeof(type pointed to)
            else 
            {
                int size = size_of_type(resolve_final_ptr_type(program->reassignment_node.type, 1));
                switch(program->reassignment_node.op)
                {
                    case TOKEN_ASSIGN:
                    {
                        fprintf(asm_file, "    mov %s [%s], %s \n", 
                            getWordForSize(rvalue.linkedToRegister->size), lvalue.linkedToRegister->name, rvalue.linkedToRegister->name);
                        break;
                    }

                    case TOKEN_ADD_ASSIGN:
                    {
                        fprintf(asm_file, "    imul %s, %i \n", rvalue.linkedToRegister->name, size );
                        fprintf(asm_file, "    add [%s], %s \n", lvalue.linkedToRegister->name, rvalue.linkedToRegister->name);
                        break;
                    }

                    case TOKEN_SUB_ASSIGN:
                    {
                        fprintf(asm_file, "    sub [%s], %s \n", lvalue.linkedToRegister->name, rvalue.linkedToRegister->name);
                        break;
                    }

                    case TOKEN_MUL_ASSIGN:
                    {
                        
                        break;
                    }

                    case TOKEN_MOD_ASSIGN:
                    {
                        break;
                    }

                    case TOKEN_DIV_ASSIGN:
                    {
                        break;
                    }

                }
            }

            


            //lvalue = with_lvalue_context(asm_file, program->reassignment_node.lvalue, current_st, current_ft);
            //rvalue = with_lvalue_context(asm_file, program->reassignment_node.expression, current_st, current_ft);
            //emit_reassign(asm_file,lvalue.linkedToRegister,rvalue.linkedToRegister, &program->reassignment_node);



>>>>>>> Stashed changes
            
            freeRegister(result.linkedToRegister);
            return none;
        }



        case NODE_DECLARATION:
        {
            Operand none = {.type = NONE};
            // Done in the first pass 
            if (get_current_bt_context() == CTX_NONE)
            {
                return none;
            }
            
            // Symbol *identifier = lookup_identifier(current_st, bt_context.current_scope, program->declaration_node.identifier);

<<<<<<< Updated upstream
            fprintf(asm_file, "    mov %s [rbp - %i], 0\n", getWordForSize(size_of_type(identifier->type)), 
                    1 + identifier->offset);
=======
            // fprintf(asm_file, "    mov %s [rbp - %i], 0\n", getWordForSize(size_of_type(identifier->type)), 
            //         getStackBasePtr() - identifier->offset);

            // exit_node(program);
>>>>>>> Stashed changes
            return none;
        }

        // rax and rbx used for temporary 
        // rcx stores the result
        case NODE_BINARY_OP:
        {
            
            int left_depth = compute_binaryOperationDepth(program->binary_op_node.left, 1);
<<<<<<< Updated upstream
=======
            

>>>>>>> Stashed changes
            int right_depth = compute_binaryOperationDepth(program->binary_op_node.right, 1);
            
            //printf("Left depth: %i, right depth: %i \n", left_depth, right_depth);
            
<<<<<<< Updated upstream
=======
            Register *temp = NULL;

>>>>>>> Stashed changes
            Operand left;
            Operand right;
            if (right_depth > left_depth)
            {
                right = buildStart(asm_file, program->binary_op_node.right, current_st, current_ft);
                left = buildStart(asm_file, program->binary_op_node.left, current_st, current_ft);
            }

            else 
            {
                left = buildStart(asm_file, program->binary_op_node.left, current_st, current_ft);
                right = buildStart(asm_file, program->binary_op_node.right, current_st, current_ft);
            }

            char *opcode = strToX86Binary(program->binary_op_node.op);

            BinOPCode binOperation = strToBinOPCode(program->binary_op_node.op); 
            
<<<<<<< Updated upstream
=======
            char *movOP; 

            if (left.linkedToRegister->size == 1) { movOP = "mov"; }
            else { movOP = "movzx"; }
           
            

            //Ensure left and right are stored in registers of the same size
            //If not, promote to the smaller size (which is the one in use)
            

            promote_smaller_reg_size(&left.linkedToRegister, &right.linkedToRegister);

            printf("We are here \n");
>>>>>>> Stashed changes

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

                case DIV: 
                {
                    // Move dividend to RAX 
                    if (left.linkedToRegister != gpr->registers[0])
                    {
                        fprintf(asm_file, "    rax, %s\n", left.linkedToRegister->name);
                    }

                    // Sign extend RAX into RDX 
                    fprintf(asm_file, "    cqo\n");
                    // Perform division 
                    fprintf(asm_file, "    idiv %s\n", right.linkedToRegister->name);
                    // Result is in RAX (quotient), remainder in RDX 
                    
                    // Free right register 
                    freeRegister(right.linkedToRegister);
                    
                    // Return the result in rax 
                    Operand result = { .linkedToRegister = gpr->registers[0] };
                    return result;
                    break;
                }

                case MOD: 
                {
                     // Move dividend to RAX 
                    if (left.linkedToRegister != gpr->registers[0])
                    {
                        fprintf(asm_file, "    rax, %s\n", left.linkedToRegister->name);
                    }

                    // Sign extend RAX into RDX 
                    fprintf(asm_file, "    cqo\n");
                    // Perform division 
                    fprintf(asm_file, "    idiv %s\n", right.linkedToRegister->name);
                    // Result is in RAX (quotient), remainder in RDX 
                    
                    // Free right register 
                    freeRegister(right.linkedToRegister);
                    
                    // Return the result in RDX
                    Operand result = { .linkedToRegister = gpr->registers[3] };
                    return result;
                    break;                   
                }

                case GT: 
                {
                    Register *eightBitRegister = getDirectTemporaryRegisterForSize(1);
                    fprintf(asm_file, "    cmp %s, %s\n", left.linkedToRegister->name, right.linkedToRegister->name);
                    fprintf(asm_file, "    setg %s\n", eightBitRegister->name );
                    fprintf(asm_file, "    mov %s, %s\n", left.linkedToRegister->name, eightBitRegister->name );
                    break;
                }

                case LT: 
                {
                    Register *eightBitRegister = getDirectTemporaryRegisterForSize(1);
                    fprintf(asm_file, "    cmp %s, %s\n", left.linkedToRegister->name, right.linkedToRegister->name);
                    fprintf(asm_file, "    setl %s\n", eightBitRegister->name );
                    fprintf(asm_file, "    mov %s, %s\n", left.linkedToRegister->name, eightBitRegister->name );
                    break;               
                }

                case GTEQ:
                {
                    Register *eightBitRegister = getDirectTemporaryRegisterForSize(1);
                    fprintf(asm_file, "    cmp %s, %s\n", left.linkedToRegister->name, right.linkedToRegister->name);
                    fprintf(asm_file, "    setge %s\n", eightBitRegister->name );
                    fprintf(asm_file, "    mov %s, %s\n", left.linkedToRegister->name, eightBitRegister->name);
                    break;
                }

                case LTEQ:
                {
                    Register *eightBitRegister = getDirectTemporaryRegisterForSize(1);
                    fprintf(asm_file, "    cmp %s, %s\n", left.linkedToRegister->name, right.linkedToRegister->name);
                    fprintf(asm_file, "    setle %s\n", eightBitRegister->name);
                    fprintf(asm_file, "    mov %s, %s\n", left.linkedToRegister->name, eightBitRegister->name);
                    break;
                }

                case EQ:
                {
                    Register *eightBitRegister = getDirectTemporaryRegisterForSize(1);
                    fprintf(asm_file, "    cmp %s, %s\n", left.linkedToRegister->name, right.linkedToRegister->name);
                    fprintf(asm_file, "    sete %s\n", eightBitRegister->name);
                    fprintf(asm_file, "    mov %s, %s\n", left.linkedToRegister->name, eightBitRegister->name);
                    break;
                }

                case NEQ:
                {
                    Register *eightBitRegister = getDirectTemporaryRegisterForSize(1); 
                    fprintf(asm_file, "    cmp %s, %s\n", left.linkedToRegister->name, right.linkedToRegister->name);
                    fprintf(asm_file, "    setne %s\n", eightBitRegister->name);
                    fprintf(asm_file, "    mov %s, %s\n", left.linkedToRegister->name, eightBitRegister->name);
                    break;
                }
                
                // logical and 
                case AND:
                {
                    Register *leftEightBitRegister = getDirectTemporaryRegisterForSize(1);
                    Register *rightEightBitRegister = getDirectTemporaryRegisterForSize(1);
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
                    // Move with extension 
                    fprintf(asm_file, "    movzx %s, %s\n", left.linkedToRegister->name, leftEightBitRegister->name);
                    break;
                }
                
                // logical or
                case OR: 
                {
                    Register *leftEightBitRegister = getDirectTemporaryRegisterForSize(1);
                    Register *rightEightBitRegister = getDirectTemporaryRegisterForSize(1);
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
                    // Move with extension 
                    fprintf(asm_file, "    movzx %s, %s\n", left.linkedToRegister->name, leftEightBitRegister->name);
                    break;                
                }

                default: 
                {
                    fprintf(stderr, "Unsupported binary operation to build. Got: %i \n", binOperation );
                    exit(1);
                }
            
            }
            
<<<<<<< Updated upstream

            // Right has been asborbed in left, right is free now 
            right.linkedToRegister->is_being_used = 0;
            // Free the 64 bit register too 
            right.linkedToRegister->parent64Register->is_being_used = 0;
=======
            freeRegister(right.linkedToRegister); 
            
            exit_node(program);
            
            printf("---- GOT HERE \n");
            // if (temp != NULL) 
            // { 
            //     fprintf(asm_file, "    ;Unsaving %s \n",temp->name); 
            //     unsaveguardRegister(temp); 
            // }
>>>>>>> Stashed changes

            Operand result = { .linkedToRegister = left.linkedToRegister };
            return result;
        }

        case NODE_STDALONE_POSTFIX_OP:
        case NODE_POSTFIX_OP:
        {
            enter_node(program);

            PostfixOPCode postfixOPCode = strToPostfixOPCode(program->postfix_op_node.op);

            Operand operand = with_lvalue_context(asm_file, program->postfix_op_node.left, current_st, current_ft);

            Register *temp = getDirectTemporaryRegisterForSize(program->postfix_op_node.size_of_operand);

            int is_ptr = is_type_ptr(program->postfix_op_node.type);

            fprintf(asm_file, "    mov %s, [%s] \n", temp->name, operand.linkedToRegister->name);

            // Incrementing and decrementing one for basic types
            if (!is_ptr)
            {
                switch (postfixOPCode)
                {
                    case POSTFIX_INC:
                    {
                        fprintf(asm_file, "    inc %s [%s] \n", getWordForSize(program->postfix_op_node.size_of_operand), operand.linkedToRegister->name);
                        break;
                    }

                    case POSTFIX_DEC:
                    {
                        fprintf(asm_file, "    dec %s [%s] \n", getWordForSize(program->postfix_op_node.size_of_operand), operand.linkedToRegister->name);
                        break;
                    }
                }
            }

            // Incrementing and decrementing sizeof(type pointed to) for pointer types
            else 
            {
                int size = size_of_type(program->postfix_op_node.type);
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


            Operand result;
            result.linkedToRegister = temp;
            exit_node(program);
            return result;
        }

        case NODE_UNARY_OP:
        {
            Operand operand = buildStart(asm_file, program->unary_op_node.right, current_st, current_ft);

            UnaryOPCode unaryOPCode = strToUnaryOPCode(program->unary_op_node.op);
            switch(unaryOPCode)
            {
                case UNARY_MINUS:
                {
                    fprintf(asm_file, "    neg %s\n",  operand.linkedToRegister->name);
                    break;
                }
<<<<<<< Updated upstream
=======

                case UNARY_ADDRESS:
                {
                    emit_comment(asm_file, "Getting the address");

                    operand = with_lvalue_context(asm_file, program->unary_op_node.right, current_st, current_ft);
                    result.linkedToRegister = operand.linkedToRegister;

                    // The address is 8 bytes 
                    // Register *reg_holding_address = getGPR(8);
                    // printf("Stack base pointer = %i, Operand offset = %i \n", getStackBasePtr(), operand.offset);
                    // fprintf(asm_file, "    lea %s, [rbp - %i] \n", reg_holding_address->name, getStackBasePtr() - operand.offset);
                    // result.linkedToRegister = reg_holding_address;
                    break;
                }

                case UNARY_DEREF:
                {
                    // *ptr = *ptr + 1
                    // LVALUE = mov rax, [rbp - ptr.offset]  -> rax contains the address ptr points to 
                    // RVALUE = 
                    emit_comment(asm_file, "Dereferencing");
                    
                    if (getCurrentWorkingContext() == COMPUTING_LVALUE)
                    {
                        pushWorkingContext(COMPUTING_RVALUE);
                        operand = buildStart(asm_file, program->unary_op_node.right, current_st, current_ft);
                        result.linkedToRegister = operand.linkedToRegister;
                        popWorkingContext();
                    }

                    else 
                    {
                        expectRegister(ANY, 8);
                        operand = buildStart(asm_file, program->unary_op_node.right, current_st, current_ft);
                        clearExpectation();
                        
                        Register *reg = matchExpectedRegister();

                        fprintf(asm_file, "    mov %s, %s [%s]\n", reg->name, getWordForSize(reg->size), operand.linkedToRegister->name);
                        result.linkedToRegister = reg;
                    }

                    emit_comment(asm_file, "Finished dereferencing");
                    break;
                }

                default:
                {
                    fprintf(stderr, "Unary op code: %s, not supported yet \n", program->unary_op_node.op);
                    exit(1);
                }
>>>>>>> Stashed changes
            }

            Operand result = { .linkedToRegister = operand.linkedToRegister };
            return result;
        }

        case NODE_IDENTIFIER:
        {


            Symbol *identifier = lookup_identifier(current_st, bt_context.current_scope, program->identifier_node.name );
            
<<<<<<< Updated upstream
            Register *reg = matchExpectedRegister();
=======
            Register *reg = matchExpectedRegisterOrDefault(ANY, identifier->size);
>>>>>>> Stashed changes

            char *opcode = "mov";

            if (identifier->scope >= 1)
            {
                fprintf(asm_file, "    %s %s, [rbp - %i]\n", opcode, reg->name ,1 + identifier->offset);
            }

            else
            {
<<<<<<< Updated upstream
                fprintf(asm_file, "    %s %s, [%s]\n", opcode, reg->name, identifier->identifier);
            }

            Operand operand = {.linkedToRegister = reg}; 
=======
                fprintf(asm_file, "    mov %s, [%s]\n", reg->name, identifier->identifier);
            }
            

            exit_node(program);

            
            Operand operand = {.linkedToRegister = reg, .offset = identifier->offset}; 
>>>>>>> Stashed changes
            return operand;
        }

        case NODE_NUMBER: 
        {
            char buffer[64];
            snprintf(buffer, sizeof(buffer), "%"PRId64, program->number_node.number_value);
            Operand number = {.word = strdup(buffer) };
            number.type = NUMBER;
            Register *reg = matchExpectedRegister();
            number.linkedToRegister = reg;
            fprintf(asm_file, "    mov %s, %"PRId64"\n", reg->name, program->number_node.number_value); 
<<<<<<< Updated upstream
=======

            exit_node(program);


>>>>>>> Stashed changes
            return number;
        }

        case NODE_CHAR:
        {
            Register *reg = matchExpectedRegister();
            Operand character = {.linkedToRegister = reg};
            fprintf(asm_file, "    mov %s, %i\n", reg->name, program->char_node.char_value);
            return character;
        }
        
        default:
        {
            break;
        }
    }
}
