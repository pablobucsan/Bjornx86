


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


void initMatcher()
{
    matcher = malloc(sizeof(Matcher));
    matcher->expecation_to_match = malloc(sizeof(Expectation));
    matcher->expecation_to_match->size = 0;
    matcher->expecation_to_match->registerType = ANY;
    matcher->empty = 1;
}

void expectRegister(RegisterType registerType, int size)
{
    if (!matcher->empty)
    {
        fprintf(stderr, "Cannot expect more than one expectations at a time. \n");
        exit(1);
    }

    matcher->expecation_to_match->registerType = registerType;
    matcher->expecation_to_match->size = size;
    matcher->empty = 0;

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
}


char *short_for_type(char *type)
{
    if (strcmp(type, "uint8") == 0) { return "u8"; }
    if (strcmp(type, "uint16") == 0) { return "u16"; }
    if (strcmp(type, "uint32") == 0) { return "u32"; }
    if (strcmp(type, "int8") == 0) { return "i8"; }
    if (strcmp(type, "int16") == 0) { return "i16"; }
    if (strcmp(type, "int32") == 0) { return "i32"; }
    if (strcmp(type, "char") == 0) { return "c"; }
    if (strcmp(type, "bool") == 0) { return "b"; }
    if (strcmp(type, "str") == 0) { return "s"; }

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



void write_function_signature(FILE *asm_file, ASTNode *program)
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
    // printf("Building for node type: %s \n", astTypeToStr(program));
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

            pop_bt_context();
            break;
        }

        case NODE_FOR:
        {
            push_bt_context(CTX_FOR);

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
            
            fprintf(asm_file,"    ;Align to 16 bytes\n");
            fprintf(asm_file,"    sub rsp, %i\n", align_size);


            // Load the parameters 
            for (int i = 0; i < f->param_count; i++)
            {
                Symbol *param = lookup_identifierByIndex(f->local_symbols, i);

                // Expectate a FPR register with the size of the parameter 
                expectRegister(FPR, size_of_type(param->type));
                // Match it 
                fprintf(asm_file, "    mov [rbp - %i], %s \n", 1 + param->offset, matchExpectedRegister()->name);
            }

            
            // Clear the FPR 
            clearRegisters(fpr);

            // Instructions 
            buildStart(asm_file, program->funcdef_node.body, f->local_symbols, f->local_functions ); 

            // Unreserve the stack 
            fprintf(asm_file,"    ;Restore the stack pointer\n");
            fprintf(asm_file,"    add rsp, %i\n", align_size);
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


            fprintf(asm_file, "    ; Preparing to call function %s()\n", f->identifier);


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
            return return_value;
        }

        case NODE_RETURN:
        {
            fprintf(asm_file, "    ; Return node\n");
            // Gotta put the return value in RAX 
            Operand return_value = buildStart(asm_file, program->return_node.return_expr, current_st, current_ft);
            return return_value;
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
            
            expectRegister(ANY, size_of_type(program->assignment_node.type));
            
            Operand result = buildStart(asm_file, program->assignment_node.expression, current_st, current_ft);
            Symbol *identifier = lookup_identifier(current_st, bt_context.current_scope, program->assignment_node.identifier);
            
            fprintf(asm_file, "    mov %s [rbp - %i], %s\n", getWordForSize(size_of_type(identifier->type)),
                    1 + identifier->offset, getTruncatedRegister(result.linkedToRegister, size_of_type(identifier->type))->name); 
            // Free result register 
            freeRegister(result.linkedToRegister);

            return none;
        }

        case NODE_REASSIGNMENT:
        {
            Operand none = {.type = NONE};

            if (get_current_bt_context() == CTX_NONE)
            {
                return none;
            }
            
            Symbol *s = lookup_identifier(current_st, bt_context.current_scope ,program->reassignment_node.identifier);
            expectRegister(ANY, size_of_type(s->type));
            Operand result = buildStart(asm_file, program->reassignment_node.expression, current_st, current_ft);
            
            // Handle smart reassignments like +=, -=, *=, /= 

            fprintf(asm_file, "    mov %s [rbp - %i], %s\n", getWordForSize(size_of_type(s->type)), 
                    1 + s->offset, getTruncatedRegister(result.linkedToRegister, size_of_type(s->type))->name);
            
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
            
            Symbol *identifier = lookup_identifier(current_st, bt_context.current_scope, program->declaration_node.identifier);

            fprintf(asm_file, "    mov %s [rbp - %i], 0\n", getWordForSize(size_of_type(identifier->type)), 
                    1 + identifier->offset);
            return none;
        }

        // rax and rbx used for temporary 
        // rcx stores the result
        case NODE_BINARY_OP:
        {
            
            int left_depth = compute_binaryOperationDepth(program->binary_op_node.left, 1);
            int right_depth = compute_binaryOperationDepth(program->binary_op_node.right, 1);
            
            //printf("Left depth: %i, right depth: %i \n", left_depth, right_depth);
            
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
            

            switch(binOperation)
            {
                case ADD:
                {
                    fprintf(asm_file, "    add %s, %s\n", left.linkedToRegister->name, right.linkedToRegister->name);
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
            

            // Right has been asborbed in left, right is free now 
            right.linkedToRegister->is_being_used = 0;
            // Free the 64 bit register too 
            right.linkedToRegister->parent64Register->is_being_used = 0;

            Operand result = { .linkedToRegister = left.linkedToRegister };
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
            }

            Operand result = { .linkedToRegister = operand.linkedToRegister };
            return result;
        }

        case NODE_IDENTIFIER:
        {


            Symbol *identifier = lookup_identifier(current_st, bt_context.current_scope, program->identifier_node.name );
            
            Register *reg = matchExpectedRegister();

            char *opcode = "mov";

            if (identifier->scope >= 1)
            {
                fprintf(asm_file, "    %s %s, [rbp - %i]\n", opcode, reg->name ,1 + identifier->offset);
            }

            else
            {
                fprintf(asm_file, "    %s %s, [%s]\n", opcode, reg->name, identifier->identifier);
            }

            Operand operand = {.linkedToRegister = reg}; 
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
