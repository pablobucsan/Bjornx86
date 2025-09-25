


#include "../include/builder.h"
#include <stdio.h>
#include <stdlib.h>
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
WorkingContext workingContext;

CurrentFrameAndFunction CFF = { .stack_base_ptr = 0, .f = NULL };

void updateCFF(Function *f)
{
    CFF.f = f;
    CFF.stack_base_ptr = align_rsp(f->local_symbols->weight);
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
    
    printf("With rvalue context.... got result in %s \n", result.linkedToRegister->name);
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
    int total = (locals_weight + 8 + 15) & ~15; // Round up weight + 8 to multiple of 16
    int align_size = total - 8; // Subtract push rbp
    if (align_size < 8) align_size = 8;

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
    matcher->current_expectation_index = -1;
}

void expectRegister(RegisterType registerType, int size)
{
    matcher->current_expectation_index++;
    // printf("Expecting type: %i and size: %i. Index = %i \n", registerType, size, matcher->current_expectation_index);
    matcher->expecations_to_match[matcher->current_expectation_index]->registerType = registerType;
    matcher->expecations_to_match[matcher->current_expectation_index]->size = size;
}

void clearExpectation()
{
    if (matcher->current_expectation_index == -1)
    {
        fprintf(stderr, "There is no expectation to clear.\n");
        exit(1);
    }
    matcher->current_expectation_index--;
    // printf("Cleared expecations. Index = %i \n", matcher->current_expectation_index);
}


void clearAllExpectations()
{
    matcher->current_expectation_index = -1;
}

Register *matchExpectedRegisterOrDefault(RegisterType type, int default_size)
{
    if (matcher->current_expectation_index == -1)
    {
        switch (type) 
        {       
            case ANY:
            case GPR: return getGPR(default_size);
            case FPR: return getFPR(default_size);
            default:
            {
                fprintf(stderr, "Unknown register type to match. Got : %i \n", 
                    matcher->expecations_to_match[matcher->current_expectation_index]->registerType);
                exit(1);               
            }
        }
    }
    
    // If there 
    return matchExpectedRegister();
}


Register *matchExpectedRegister()
{
    
    if (matcher->current_expectation_index == -1)
    {
        fprintf(stderr, "No expectations to match.\n");
        exit(1);
    }
    

    switch (matcher->expecations_to_match[matcher->current_expectation_index]->registerType)
    {
        case ANY:
        case GPR: return getGPR(matcher->expecations_to_match[matcher->current_expectation_index]->size);
        case FPR: return getFPR(matcher->expecations_to_match[matcher->current_expectation_index]->size);
        default:
        {
            fprintf(stderr, "Unknown register type to match. Got : %i \n", 
                    matcher->expecations_to_match[matcher->current_expectation_index]->registerType);
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
    printf("----- CLEARING REGISTERS------- \n");

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


Register *getSubRegisterForSize(RegisterTable *rt, char *name, int size)
{
    for (int i = 0; i < rt->num_of_registers; i++)
    {
        if (strcmp(rt->registers[i]->name, name) == 0)
        {
            switch (size)
            {
                case 8: return rt->registers[i];
                case 4: return rt->registers[i]->subRegisters[0];
                case 2: return rt->registers[i]->subRegisters[1];
                case 1: return rt->registers[i]->subRegisters[2];
                default: 
                {
                    fprintf(stderr, "Cannot get a subregister for size: %i\n", size);
                    exit(1);
                }
            }
        }
    }

    fprintf(stderr, "Couldn't find a subregister of %s with size: %i in the given table\n", name, size);
    exit(1);
}

Register *getFPR(int size)
{


    for (int i = 0; i < fpr->num_of_registers; i++)
    {
        // Get a completely free register 
        if (fpr->registers[i]->is_being_used == 0) 
        {

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

    fprintf(stderr, "Can't get temporary register for size: %i. All are being used\n", size);
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

    fprintf(stderr, "Can't get temporary register for size: %i. All are being used\n", size);
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
    printf("-----PUSHING CTX : %s ------- \n", ctxToString(ctx_type));
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
    printf("-----POPPING CTX : %s ------- \n", ctxToString(bt_context.ctx_type_stack[bt_context.current_scope]));
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
    switch (node->node_type) 
    {
        case NODE_NULL:
        case NODE_NUMBER:
        case NODE_SUBSCRIPT:
        case NODE_FUNC_CALL:
        case NODE_IDENTIFIER: return depth;
        

        case NODE_UNARY_OP: return depth + 1;
    }


    BinaryOPNode *binary_op_node = (BinaryOPNode*)node;

    int sub_left = compute_binaryOperationDepth(binary_op_node->left, depth + 1);
    int sub_right = compute_binaryOperationDepth(binary_op_node->right, depth + 1);
}


char *short_for_type(char *type)
{
    // Builtin types 
    if (strcmp(type, "uint8") == 0) { return "u8"; }
    if (strcmp(type, "uint16") == 0) { return "u16"; }
    if (strcmp(type, "uint32") == 0) { return "u32"; }
    if (strcmp(type, "int8") == 0) { return "i8"; }
    if (strcmp(type, "int16") == 0) { return "i16"; }
    if (strcmp(type, "int32") == 0) { return "i32"; }
    if (strcmp(type, "char") == 0) { return "c"; }
    if (strcmp(type, "bool") == 0) { return "b"; }
    if (strcmp(type, "str") == 0) { return "s"; }
    
    
    // User-defined types, gotta check that later i guess, maybe just the type itself is good enough

    else { return type; }
}

char *write_loop_signature(FILE *asm_file, ASTNode *program)
{
    char *label = malloc(100 * sizeof(char));
    switch (program->node_type)
    {
        case NODE_WHILE:
        {
            fprintf(asm_file, "\n;While loop\n");
            fprintf(asm_file, ".while_loop_%i:\n", bt_context.while_block_id);
            sprintf(label, ".while_loop_%i", bt_context.while_block_id);
            break;
        }

        case NODE_FOR:
        {
            fprintf(asm_file, "\n;For loop\n");
            fprintf(asm_file,".for_loop_%i:\n", bt_context.for_block_id);
            sprintf(label, ".for_loop_%i", bt_context.for_block_id);
            break;
        }
    }

    return label;
}


char *write_if_signature(FILE *asm_file)
{
    char *label = malloc(100 * sizeof(char));
    
    fprintf(asm_file, "\n;If statement\n");
    fprintf(asm_file, ".if_stmt_%i:\n", bt_context.if_block_id);
    sprintf(label, ".if_stmt_%i", bt_context.if_block_id);

    return label;

}


char *getRetLabelFromFunction(Function *f)
{
    char **param_types = get_paramTypes_from_Params(f->params, f->param_count);
    char *ret_label = malloc(sizeof(char) * 200);


    sprintf(ret_label, ".ret_from_%s", f->identifier);
   

    for (int i = 0; i < f->param_count; i++)
    {
        char *buffer = malloc(sizeof(char) * 32);
        sprintf(buffer, "_%s", short_for_type(param_types[i]));
        strcat(ret_label, buffer);
        free(buffer);
    }
    
// ret_label = realloc(ret_label, strlen(ret_label));
    return ret_label;

}

char *write_function_signature(FILE *asm_file, ASTNode *program)   
{

    char **param_types;
    char *func_signature = malloc(sizeof(char) * 200);
    switch(program->node_type)
    {
        case NODE_EXTERN_FUNC_DEF:
        {
            fprintf(asm_file, "extern _%s", program->extern_func_def_node.func_name);
            sprintf(func_signature, "%s", program->extern_func_def_node.func_name);
            param_types = get_paramTypes_from_Params(program->extern_func_def_node.params, program->extern_func_def_node.params_count);
            for (int i = 0; i < program->extern_func_def_node.params_count; i++)
            {

                char *buffer = malloc(sizeof(char) * 32);
                sprintf(buffer, "_%s", short_for_type(param_types[i]));
                fprintf(asm_file, "_%s", short_for_type(param_types[i])); 
                strcat(func_signature, buffer );
                free(buffer);
            }
            fprintf(asm_file, "\n");
            break;
        }

        case NODE_STDALONE_FUNC_CALL:
        {
            fprintf(asm_file, "    call _%s", program->stdalone_func_call_node.identifier);
            sprintf(func_signature, "%s", program->stdalone_func_call_node.identifier);
            param_types = program->stdalone_func_call_node.params_type;
            for (int i = 0; i < program->stdalone_func_call_node.params_count; i++)
            {
                char *buffer = malloc(sizeof(char) * 32);
                buffer[0] = '_';
                strcat(buffer, short_for_type(param_types[i]));

                fprintf(asm_file, "_%s", short_for_type(param_types[i])); 
                strcat(func_signature, buffer );
                free(buffer);            
            }
            fprintf(asm_file, "\n");
            break;
        }

        case NODE_FUNC_CALL:
        {
            fprintf(asm_file, "    call _%s", program->funccall_node.identifier);
            sprintf(func_signature, "%s", program->funccall_node.identifier);
            param_types = program->funccall_node.params_type;
            for (int i = 0; i < program->funccall_node.params_count; i++)
            {
                char *buffer = malloc(sizeof(char) * 32);
                buffer[0] = '_';
                strcat(buffer, short_for_type(param_types[i]));

                fprintf(asm_file, "_%s", short_for_type(param_types[i])); 
                strcat(func_signature, buffer );
                free(buffer);
            }
            fprintf(asm_file, "\n");
            break;
        }

        case NODE_FUNC_DEF:
        {
            fprintf(asm_file, "_%s", program->funcdef_node.func_name);
            sprintf(func_signature, "%s", program->funcdef_node.func_name);
            param_types = get_paramTypes_from_Params(program->funcdef_node.params, program->funcdef_node.params_count);
            for (int i = 0; i < program->funcdef_node.params_count; i++)
            {
                char *buffer = malloc(sizeof(char) * 32);
                buffer[0] = '_';
                strcat(buffer, short_for_type(param_types[i]));

                fprintf(asm_file, "_%s", short_for_type(param_types[i])); 
                strcat(func_signature, buffer );
                free(buffer);
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
    

    func_signature = realloc(func_signature, strlen(func_signature));
    return func_signature;
 
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
        
        /*
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
        */

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
    if (program == NULL)
    {
        fprintf(stderr, "Trying to build for a NULL node \n");
        exit(1);
    }
    printf("Building for node type: %s \n", astTypeToStr(program));
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
            Operand none = {.type = NONE};
            return none;
        }

        case NODE_USE_DIRECTIVE:
        {
            enter_node(program);

            buildStart(asm_file, program->use_node.program, current_st, current_ft);

            exit_node(program);
            Operand none = {.type = NONE};
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
            char *label = write_loop_signature(asm_file, program);
            char *end = malloc(120 * sizeof(char));
            sprintf(end, ".end_%s", label);

            // Condition check
            fprintf(asm_file, "    ;Condition check\n");
            Operand condition = buildStart(asm_file, program->while_node.condition_expr, current_st, current_ft);
            freeRegister(condition.linkedToRegister);
            // printf("Condition operand: Linked to: %s\n", condition.linkedToRegister);
            fprintf(asm_file, "    cmp %s, 0\n", condition.linkedToRegister->name);
            fprintf(asm_file, "    je %s\n",end);


            fprintf(asm_file,"    ;Body\n");
            buildStart(asm_file, program->while_node.body, current_st, current_ft);


            fprintf(asm_file, "    ;Back to condition check\n");
            fprintf(asm_file, "    jmp %s\n\n", label);
            fprintf(asm_file, "%s:\n", end);
            
            exit_node(program);
            break;
        }

        case NODE_FOR:
        {
            enter_node(program);

            // Initial assignment 
            fprintf(asm_file, "\n    ;Initial assignment of for loop\n");
            buildStart(asm_file, program->for_node.assignment_expr, current_st, current_ft);

            char *label = write_loop_signature(asm_file, program);
            char *end = malloc(120 * sizeof(char));
            sprintf(end, ".end_%s", label);
            
            // Condition check 
            fprintf(asm_file, "    ;Condition check\n");
            Operand condition = buildStart(asm_file, program->for_node.condition_expr, current_st, current_ft);
            freeRegister(condition.linkedToRegister);
            fprintf(asm_file, "    cmp %s, 0\n", condition.linkedToRegister->name);
            fprintf(asm_file, "    je %s\n", end);

            // Body
            fprintf(asm_file, "    ;Body\n");
            buildStart(asm_file, program->for_node.body, current_st, current_ft);
            // Reassignment 
            fprintf(asm_file,"    ;Reassignment for index\n");
            buildStart(asm_file, program->for_node.reassignment_expr, current_st, current_ft);
            // Back to condition check 
            fprintf(asm_file,"    ;Back to condition check\n");
            fprintf(asm_file,"    jmp %s\n\n", label);
            fprintf(asm_file,"%s:\n", end);
            
            exit_node(program);
            break;
        }

        case NODE_IF: 
        {
            // printf("Here\n");
            enter_node(program);
            
            if (program->if_node.else_body != NULL) {  increment_bt_ctx_block_id(CTX_ELSE); }

            char *if_label = write_if_signature(asm_file);
            char *else_label = malloc(120 * sizeof(char));
            sprintf(else_label, ".else_stmt_%i", bt_context.else_block_id);
            char *end = malloc(120 * sizeof(char));
            sprintf(end, ".end_%s", if_label);
            
            
            // Condition 
            expectRegister(GPR, program->if_node.condition_size);
            Operand condition = buildStart(asm_file, program->if_node.condition_expr, current_st, current_ft);
            freeRegister(condition.linkedToRegister);
            clearExpectation();

            fprintf(asm_file, "    cmp %s, 0\n", condition.linkedToRegister->name);
            if (program->if_node.else_body != NULL)
            {

                printf("####################### ELSE BODY IS NOT NULL ############################## \n");
                fprintf(asm_file, "    je %s\n", else_label);
            }

            else 
            {

                printf("####################### ELSE BODY IS NULL ############################## \n");
                fprintf(asm_file,"    je %s\n", end); 
            }
            
            //If Body 
            printf("----- ABOUT TO BUILD IF'S BODY ------ \n");
            buildStart(asm_file, program->if_node.body, current_st, current_ft);
            printf("------- DONE ----- \n");
            fprintf(asm_file, "    jmp %s\n", end);

            //Else Body 
            if (program->if_node.else_body != NULL)
            {
                fprintf(asm_file, "%s:\n", else_label);
                buildStart(asm_file, program->if_node.else_body, current_st, current_ft);
            }
            
            fprintf(asm_file, "%s:\n", end);
            exit_node(program);
            break; 
        }


        case NODE_FUNC_DEF:
        {
            
            enter_node(program);

            char *func_signature = write_function_signature(asm_file, program);
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
            
            updateCFF(f);
                      
            fprintf(asm_file,"    ;Align to 16 bytes\n");
            fprintf(asm_file,"    sub rsp, %i\n", getStackBasePtr());


            // Load the parameters 
            for (int i = 0; i < f->param_count; i++)
            {
                Symbol *param = lookup_identifierByIndex(f->local_symbols, i);

                // Expectate a FPR register with the size of the parameter 
                expectRegister(FPR, size_of_type(get_base_type(param->type)));
                // Match it 
                fprintf(asm_file, "    mov [rbp - %i], %s \n", 
                        getStackBasePtr() - param->offset, matchExpectedRegister()->name);
                clearExpectation();
            }

             
            // Instructions 
            buildStart(asm_file, program->funcdef_node.body, f->local_symbols, f->local_functions ); 

            // Unreserve the stack 
            fprintf(asm_file,"    ;Restore the stack pointer and return\n");
            fprintf(asm_file,".ret_from_%s:\n", func_signature);
            fprintf(asm_file,"    add rsp, %i\n", getStackBasePtr());
            // Restore RBP 
            fprintf(asm_file,"    pop rbp\n");
            // Return (If 7 or more arguments used, gotta use ret <N> where N = # args in stack x bytes each)
            fprintf(asm_file,"    ret\n\n");
            
            exit_node(program);
            break;
        }
        
        case NODE_STDALONE_FUNC_CALL:
        case NODE_FUNC_CALL:
        {

            enter_node(program);
            // Get the function we are calling 
            Function *f = lookup_function(current_ft, bt_context.current_scope, 
                                          program->funccall_node.identifier, program->funccall_node.params_type, program->funccall_node.params_count);

            fprintf(asm_file, "    ; Preparing to call function %s()\n", f->identifier);

            Operand par;
            // Build the expressions for the parameters 
            for (int i = 0; i < program->funccall_node.params_count; i++)
            {
                par = with_rvalue_context(asm_file, program->funccall_node.params_expr[i], FPR, 
                                    size_of_type(get_base_type(program->funccall_node.params_type[i])), current_st, current_ft);
            }
            
            write_function_signature(asm_file, program);
            
            
            exit_node(program);
            Operand return_value;
            if (strcmp(f->rt_type, "void") != 0)
            {
                //Return value is always in RAX 
                return_value.linkedToRegister = getTruncatedRegister(gpr->registers[0], size_of_type(get_base_type(f->rt_type)));
                busyRegister(return_value.linkedToRegister);
                return return_value;
            }

            return_value.type = NONE;
            
            return return_value;
        }

        case NODE_RETURN:
        {
            printf("$$$$$$$$$$$ IN NODE RETURN $$$$$$$$$$$$$$ \n");
            enter_node(program);
            fprintf(asm_file, "    ; Return node\n");
            Operand ret;

            char *ret_label = getRetLabelFromFunction(getCurrentFunction());
            // Gotta put the return value in RAX 
            if (program->return_node.return_expr == NULL)
            {
                fprintf(asm_file, "    jmp %s\n", ret_label);
                ret.type = NONE;
                exit_node(program);
                return ret;
            }
            
            // Return size must match size declared in return type
            expectRegister(GPR, size_of_type(get_base_type(getCurrentFunction()->rt_type)));
            ret = buildStart(asm_file, program->return_node.return_expr, current_st, current_ft);
            clearExpectation();
                
            // Ensure return is in RAX 
            if (! ((ret.linkedToRegister->parent64Register == NULL && strcmp(ret.linkedToRegister->name, "rax") == 0) 
                || (ret.linkedToRegister->parent64Register != NULL && strcmp(ret.linkedToRegister->parent64Register->name, "rax") == 0)))
            {
                
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

            fprintf(asm_file,"\n    ;Array init\n");
            // Get the expected register 
            Symbol *arr = lookup_identifier(current_st, bt_context.current_scope, program->array_init_node.arr_name);
            char *base_type_arr = get_base_type(arr->type);
            int size_of_every_element = size_of_type(base_type_arr); 

            Register *first_element = NULL;
            
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
            
            fprintf(asm_file, "    ;Array init end\n\n");
            
            exit_node(program);
            Operand result = {.linkedToRegister = first_element};
            return result;
        }

        case NODE_SUBSCRIPT: 
        {
            enter_node(program);

            fprintf(asm_file, "\n    ;Array subscript\n");
            
                       
            Symbol *base_symbol = lookup_identifier(current_st, bt_context.current_scope, program->subscript_node.base_identifier);
            int size_of_base = size_of_type(get_base_type(base_symbol->type));
                
            expectRegister(GPR, 8);
            Register *base_reg = matchExpectedRegister();
            // Load base address 
            fprintf(asm_file,"    lea %s, [rbp - %i]\n", base_reg->name, getStackBasePtr() - base_symbol->offset);
            clearExpectation();

            // expectRegister(GPR, program->subscript_node.index_size);
            // Compute index 
            expectRegister(GPR, size_of_base);
            pushWorkingContext(COMPUTING_RVALUE);
            Operand index = buildStart(asm_file, program->subscript_node.index, current_st, current_ft);
            popWorkingContext();
            clearExpectation();

            // clearExpectation();
            fprintf(asm_file, "    imul %s, %i \n", index.linkedToRegister->name, size_of_base);
            fprintf(asm_file, "    movzx %s, %s \n", index.linkedToRegister->parent64Register->name, index.linkedToRegister->name); 
            fprintf(asm_file, "    add %s, %s \n", base_reg->name, index.linkedToRegister->parent64Register->name);
            


            freeRegister(index.linkedToRegister);
            Register *result = base_reg;
            // If computing RVALUE, we want the value, not the address 
            if (getCurrentWorkingContext() == COMPUTING_RVALUE) 
            {

                result = matchExpectedRegister();
                fprintf(asm_file, "    mov %s, [%s]\n", result->name, base_reg->name);
                freeRegister(base_reg);
            }

            fprintf(asm_file, "    ;Array subscript end\n\n");
            exit_node(program);
            Operand efective_address = {.linkedToRegister = result };
            return efective_address;
           
        }

        case NODE_ASSIGNMENT:
        {

            enter_node(program);
            Operand none = {.type = NONE};
            // Early exit if the assignment is in no contex.
            // That means is global, which was done in the first pass
            if (get_current_bt_context() == CTX_NONE)
            {
                return none;
            }
            
            int size = size_of_type(get_base_type(program->assignment_node.type));
            
            Operand rvalue = with_rvalue_context(asm_file, program->assignment_node.expression, GPR, size, current_st, current_ft );

            Symbol *identifier = lookup_identifier(current_st, bt_context.current_scope, program->assignment_node.identifier);
            

            // Avoid extra mov operations for array assignments. Only escalar assignments use this 
            if (program->assignment_node.expression->node_type != NODE_ARRAY_INIT)
            {
                fprintf(asm_file, "    mov %s [rbp - %i], %s\n", getWordForSize(size),
                    getStackBasePtr() - identifier->offset, getTruncatedRegister(rvalue.linkedToRegister, size)->name); 
            }
           

            exit_node(program);
            return none;
        }

        

        case NODE_REASSIGNMENT:
        {

            enter_node(program);
            // expectRegister(ANY, 2);
            Operand none = {.type = NONE};

            if (get_current_bt_context() == CTX_NONE)
            {
                return none;
            }
            
            Operand lvalue = with_lvalue_context(asm_file, program->reassignment_node.lvalue, current_st, current_ft);
            Operand rvalue = with_rvalue_context(asm_file, program->reassignment_node.expression, GPR, program->reassignment_node.size, current_st, current_ft);
           // Handle smart reassignments like +=, -=, *=, /= 
            
            fprintf(asm_file, "    mov %s [%s], %s \n", 
                    getWordForSize(rvalue.linkedToRegister->size), lvalue.linkedToRegister->name, rvalue.linkedToRegister->name);
            
            exit_node(program); 
            // clearExpectation();
            return none;
        }

        case NODE_DECLARATION:
        {
            enter_node(program);

            Operand none = {.type = NONE};
            // Done in the first pass 
            if (get_current_bt_context() == CTX_NONE)
            {
                return none;
            }
            
            Symbol *identifier = lookup_identifier(current_st, bt_context.current_scope, program->declaration_node.identifier);

            fprintf(asm_file, "    mov %s [rbp - %i], 0\n", getWordForSize(size_of_type(identifier->type)), 
                    getStackBasePtr() - identifier->offset);

            exit_node(program);
            return none;
        }
        

        // rax and rbx used for temporary 
        // rcx stores the result
        case NODE_BINARY_OP:
        {
            enter_node(program);
            

            int left_depth = compute_binaryOperationDepth(program->binary_op_node.left, 1);

            int right_depth = compute_binaryOperationDepth(program->binary_op_node.right, 1);
            //printf("Left depth: %i, right depth: %i \n", left_depth, right_depth);
           
            
            
           Register *temp = NULL;

            Operand left;
            Operand right;

            if (right_depth > left_depth)
            {
                right = buildStart(asm_file, program->binary_op_node.right, current_st, current_ft);
                // If both are func calls, we need to temporary store the first result somewhere else so the second call doesnt overwrite RAX 
                if (program->binary_op_node.left && program->binary_op_node.right && 
                        program->binary_op_node.left->node_type == NODE_FUNC_CALL && program->binary_op_node.right->node_type == NODE_FUNC_CALL)
                {

                    temp = getDirectTemporaryRegisterForSize(right.linkedToRegister->size);
                    fprintf(asm_file, "    mov %s, %s\n", temp->name, right.linkedToRegister->name);
                }
            
                // fprintf(asm_file, "    push %s\n", right.linkedToRegister->name);
                left = buildStart(asm_file, program->binary_op_node.left, current_st, current_ft);
            }

            else 
            {
                left = buildStart(asm_file, program->binary_op_node.left, current_st, current_ft);
                // fprintf(asm_file, "    push %s\n", left.linkedToRegister->name);
                if (program->binary_op_node.left && program->binary_op_node.right &&
                        program->binary_op_node.left->node_type == NODE_FUNC_CALL && program->binary_op_node.right->node_type == NODE_FUNC_CALL)
                {

                    temp = getDirectTemporaryRegisterForSize(left.linkedToRegister->size);
                    fprintf(asm_file, "    mov %s, %s\n", temp->name, left.linkedToRegister->name);
                } 
                right = buildStart(asm_file, program->binary_op_node.right, current_st, current_ft);
            }
            

            if (temp != NULL) { freeRegister(left.linkedToRegister); left.linkedToRegister = temp; }

            // Push left result to stack            
            char *opcode = strToX86Binary(program->binary_op_node.op);

            BinOPCode binOperation = strToBinOPCode(program->binary_op_node.op); 
            
            char *movOP; 

            if (left.linkedToRegister->size == 1) { movOP = "mov"; }
            else { movOP = "movzx"; }
           


            switch(binOperation)
            {
                case ADD:
                {
                    fprintf(asm_file, "    add %s, %s\n", left.linkedToRegister->name, right.linkedToRegister->name);

                    printf("Built both left and right \n");
                    break;
                }

                case MUL: 
                {
                    fprintf(asm_file, "    imul %s, %s\n", left.linkedToRegister->name, right.linkedToRegister->name);
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
                    right.linkedToRegister->is_being_used = 0;
                    
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
                    right.linkedToRegister->is_being_used = 0;
                    
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
                    fprintf(asm_file, "    %s %s, %s\n", movOP, left.linkedToRegister->name, eightBitRegister->name );
                    break;
                }

                case LT: 
                {
                    Register *eightBitRegister = getDirectTemporaryRegisterForSize(1);
                    fprintf(asm_file, "    cmp %s, %s\n", left.linkedToRegister->name, right.linkedToRegister->name);
                    fprintf(asm_file, "    setl %s\n", eightBitRegister->name );
                    fprintf(asm_file, "    %s %s, %s\n",movOP, left.linkedToRegister->name, eightBitRegister->name );
                    break;               
                }

                case GTEQ:
                {
                    Register *eightBitRegister = getDirectTemporaryRegisterForSize(1);
                    fprintf(asm_file, "    cmp %s, %s\n", left.linkedToRegister->name, right.linkedToRegister->name);
                    fprintf(asm_file, "    setge %s\n", eightBitRegister->name );
                    fprintf(asm_file, "    %s %s, %s\n", movOP, left.linkedToRegister->name, eightBitRegister->name);
                    break;
                }

                case LTEQ:
                {
                    Register *eightBitRegister = getDirectTemporaryRegisterForSize(1);
                    fprintf(asm_file, "    cmp %s, %s\n", left.linkedToRegister->name, right.linkedToRegister->name);
                    fprintf(asm_file, "    setle %s\n", eightBitRegister->name);
                    fprintf(asm_file, "    %s %s, %s\n",movOP, left.linkedToRegister->name, eightBitRegister->name);
                    break;
                }

                case EQ:
                {
                    Register *eightBitRegister = getDirectTemporaryRegisterForSize(1);
                    fprintf(asm_file, "    cmp %s, %s\n", left.linkedToRegister->name, right.linkedToRegister->name);
                    fprintf(asm_file, "    sete %s\n", eightBitRegister->name);
                    fprintf(asm_file, "    %s %s, %s\n",movOP, left.linkedToRegister->name, eightBitRegister->name);
                    break;
                }

                case NEQ:
                {
                    Register *eightBitRegister = getDirectTemporaryRegisterForSize(1); 
                    fprintf(asm_file, "    cmp %s, %s\n", left.linkedToRegister->name, right.linkedToRegister->name);
                    fprintf(asm_file, "    setne %s\n", eightBitRegister->name);
                    fprintf(asm_file, "    %s %s, %s\n", movOP, left.linkedToRegister->name, eightBitRegister->name);
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
                    fprintf(asm_file, "    %s %s, %s\n", movOP, left.linkedToRegister->name, leftEightBitRegister->name);
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
                    fprintf(asm_file, "    %s %s, %s\n", movOP, left.linkedToRegister->name, leftEightBitRegister->name);
                    break;                
                }

                default: 
                {
                    fprintf(stderr, "Unsupported binary operation to build. Got: %i \n", binOperation );
                    exit(1);
                }
            
            }
            
            printf("About to return from BINOP\n");
            // freeRegister(right.linkedToRegister); 
            
            exit_node(program);
            

            Operand result = { .linkedToRegister = left.linkedToRegister };
            return result;
        }

        case NODE_UNARY_OP:
        {

            enter_node(program);
            Operand operand = buildStart(asm_file, program->unary_op_node.right, current_st, current_ft);

            UnaryOPCode unaryOPCode = strToUnaryOPCode(program->unary_op_node.op);
            switch(unaryOPCode)
            {
                case UNARY_MINUS:
                {
                    fprintf(asm_file, "    neg %s\n",  operand.linkedToRegister->name);
                    break;
                }
            }
            
            exit_node(program);
            Operand result = { .linkedToRegister = operand.linkedToRegister };
            return result;
        }

        case NODE_IDENTIFIER:
        {

            enter_node(program);

            Symbol *identifier = lookup_identifier(current_st, bt_context.current_scope, program->identifier_node.name );
            
            Register *reg = matchExpectedRegisterOrDefault(ANY, size_of_type(get_base_type(identifier->type)));

            char *opcode = getLeaOrMovBasedOnContext();

            if (identifier->scope >= 1)
            {
                fprintf(asm_file, "    %s %s, [rbp - %i]\n", opcode, reg->name, getStackBasePtr() - identifier->offset);
            }

            else
            {
                fprintf(asm_file, "    %mov %s, [%s]\n", reg->name, identifier->identifier);
            }
            
            exit_node(program);
            Operand operand = {.linkedToRegister = reg}; 
            return operand;
        }

        case NODE_NUMBER: 
        {
            enter_node(program);

            char buffer[64];
            snprintf(buffer, sizeof(buffer), "%"PRId64, program->number_node.number_value);
            Operand number = {.word = strdup(buffer) };
            number.type = NUMBER;
            Register *reg = matchExpectedRegisterOrDefault(ANY, size_of_type(program->number_node.int_type));
            number.linkedToRegister = reg;
            fprintf(asm_file, "    mov %s, %"PRId64"\n", reg->name, program->number_node.number_value); 

            exit_node(program);

            return number;
        }

        case NODE_CHAR:
        {
            enter_node(program);

            Register *reg = matchExpectedRegisterOrDefault(ANY, 1);
            Operand character = {.linkedToRegister = reg};
            fprintf(asm_file, "    mov %s, %i\n", reg->name, program->char_node.char_value);

            exit_node(program);
            return character;
        }
        
        default:
        {
            break;
        }
    }
}
