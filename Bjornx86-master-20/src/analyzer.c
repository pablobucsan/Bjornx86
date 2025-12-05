//
// Created by pablo on 28/02/2025.
//

#include "../include/analyzer.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <inttypes.h>

SymbolTable *gb_symbolTable = NULL;
FunctionTable *gb_functionTable = NULL;
ObjectTable *gb_objectTable = NULL;
EnumTable *gb_enumTable = NULL;
StringTable *stringTable = NULL;


int func_id = 0;

Context context = {
    .current_scope = 0,
    .stack_index = 0,
    .if_block_id = 0,
    .else_block_id = 0,
    .for_block_id = 0,
    .foreach_block_id = 0,
    .while_block_id = 0,
    .function_block_id = 0,
    .ctx_type_stack = {CTX_NONE},
};




/** -------------------------------------------- LOWERERS ------------------------------- */


// Must clone re-used pointers
ASTNode *lowerForeach(ASTNode *ast_foreach)
{   

    char *iterator_name = strdup(ast_foreach->foreach_node.foreach_declaration->declaration_node.identifier);
    /**
     * uint32 counter = lower_limit;
     */
    ASTNode *ast_counter_assignment = makeAssignment("uint32", ast_foreach->foreach_node.counter_name, ast_foreach->foreach_node.lower_limit_expr);

    /**
     * Type iterator;
     */
    ASTNode *ast_iterator_id = makeIdentifier(iterator_name);
    ASTNode *ast_iterator_decl = makeDeclaration(ast_foreach->foreach_node.iterator_type, iterator_name);


    /**
     * Identifier for the counter
     * counter < upper_limit
     */

    ASTNode *counter_id_binop = makeIdentifier(ast_foreach->foreach_node.counter_name);
    ASTNode *condition = makeBinOP(counter_id_binop, ast_foreach->foreach_node.upper_limit_expr, "<");

    /**
     * 
     * iterator = iterable_expr[counter];
     */
    ASTNode *counter_id_subscript = makeIdentifier(ast_foreach->foreach_node.counter_name);

    //printf("FDHFSKDF %s\n", ast_foreach->foreach_node.iterator_type);
    //printf("YOO \n");
    ASTNode *ast_subscript = makeSubscript(ast_foreach->foreach_node.iterable_expr, counter_id_subscript, ast_foreach->foreach_node.iterable_expr_type,
                                            size_of_type(ast_foreach->foreach_node.iterator_type),4);

    //printf("HERE\n");
    ASTNode *iterator_reassignment = makeReassignment(ast_iterator_id, TOKEN_ASSIGN, ast_subscript, 
                                                        size_of_type(ast_foreach->foreach_node.iterator_type),ast_foreach->foreach_node.iterator_type );

    /**
     * counter++;
     */
    ASTNode *counter_id_incr = makeIdentifier(ast_foreach->foreach_node.counter_name);
    ASTNode *ast_incr_counter = makeStdalonePostFix(counter_id_incr, "uint32", 4, "++");
    
    /**
     * [iterator_reassignment, foreach_body, incr_counter]
     */
    int n_foreach_statements = ast_foreach->foreach_node.body->block_node.statement_count;
    ASTNode **while_statements = malloc(sizeof(ASTNode*) * (n_foreach_statements + 2));
    while_statements[0] = iterator_reassignment;
    for (int i = 1; i < n_foreach_statements + 1; i++)
    {
        while_statements[i] = cloneAST(ast_foreach->foreach_node.body->block_node.statements[i - 1]);
    }

    while_statements[n_foreach_statements + 1] = ast_incr_counter;

    /**
     * Body of the "while"
     * [iterator_reassignment, foreach_body, incr_counter]
     */
    ASTNode *ast_while_block = makeBlock(while_statements, n_foreach_statements + 2, n_foreach_statements + 2);
    ASTNode *ast_while = makeWhile(condition, ast_while_block);

    /**
     * The whole thing
     * [ast_counter_assignment, ast_iterator_decl, ast_while]
     */

    ASTNode **lowered_foreach_statements = malloc(sizeof(ASTNode*) * 3);
    lowered_foreach_statements[0] = ast_counter_assignment;
    lowered_foreach_statements[1] = ast_iterator_decl;
    lowered_foreach_statements[2] = ast_while;

    ASTNode *lowered_foreach = makeBlock(lowered_foreach_statements, 3, 3);

    // printf("N OF STATEMENTS IN FOREACH: %i\n",n_foreach_statements);
    // printf("N OF STATEMENTS IN WHILE: %i\n", n_foreach_statements + 2);
    // printf("=================== LOWERED FOREACH ========= \n");
    // print_ast(lowered_foreach, 0);
    // printf("=================== END LOWERED FOREACH ========= \n");


    return lowered_foreach;
}       

/** -------------------------------------------- LOWERERS ------------------------------- */




void reset_ctx()
{
    context.current_scope = 0;
    context.stack_index = 0;
    context.if_block_id = 0;
    context.else_block_id = 0;
    context.for_block_id = 0;
    context.foreach_block_id = 0;
    context.while_block_id = 0;
    context.function_block_id = 0;
}


void increment_ctx_block_id(ContextType ctx_type)
{
    switch (ctx_type) 
    {
        case CTX_IF: context.if_block_id++; return;
        case CTX_ELSE: context.else_block_id++; return;
        case CTX_FOR: context.for_block_id++; return;
        case CTX_FOREACH: context.foreach_block_id++; return;
        case CTX_WHILE: context.while_block_id++; return;
        case CTX_ENUM: context.enum_block_id++; return;
        case CTX_OBJECT: context.object_block_id++; return;
        case CTX_FUNCTION: context.function_block_id++; return;
        case CTX_NONE:
        {
            fprintf(stderr, "Cannot increment NONE_block_id as there is only one global scope. \n");
            exit(1);
        }
    }
}


void contextPushesScope(int *scope)
{
    ContextType current_ctx = get_current_context();


    switch(current_ctx)
    {
        case CTX_FUNCTION:
        case CTX_FOR:
        case CTX_WHILE:
        case CTX_FOREACH:
        case CTX_IF:
        case CTX_ELSE:
        {
            (*scope)++;
            break;
        }

        default: return;
    }
}

void contextPopsScope(int *scope)
{
    ContextType current_ctx = get_current_context();

    //printf("Current context to pop: %s \n", ctxToString(current_ctx));
    switch(current_ctx)
    {
        case CTX_FUNCTION:
        case CTX_FOR:
        case CTX_WHILE:
        case CTX_FOREACH:
        case CTX_IF:
        case CTX_ELSE:
        {
            (*scope)--;
            break;
        }

        default: return;
    }
}

void push_context(ContextType ctx_type)
{

    //printf("Checking whether %s is allowed in %s\n", ctxToString(ctx_type), ctxToString(get_current_context()));

    if (context.stack_index + 1 >= MAX_NESTING)
    {
        fprintf(stderr, "Cannot push another context (%s), max nesting is %i \n", 
                ctxToString(ctx_type) ,MAX_NESTING);
    }

    int allowed = isAllowedContext(ctx_type);

    if (allowed == 0)
    {
        fprintf(stderr, "Cannot push the context: %s within context of type: %s.\n", 
                ctxToString(ctx_type), ctxToString(get_current_context()));
        exit(1);
    }
    

    // Increment the block id and put the context on its right stack position
    increment_ctx_block_id(ctx_type);
    context.stack_index++;
    context.ctx_type_stack[context.stack_index] = ctx_type;

    // Increment the scope if applicable
    contextPushesScope(&context.current_scope);

    //printf("SCOPE IS NOW: %i \n", context.current_scope);
}

void pop_context()
{
    if (context.stack_index - 1 < 0)
    {
        fprintf(stderr, "Can't pop context if context is CTX_NONE. \n");
        exit(1);
    }

    contextPopsScope(&context.current_scope);

    context.stack_index--;
}

ContextType get_current_context()
{
    return context.ctx_type_stack[context.stack_index];
}



int isAllowedContext(ContextType ctx_type)
{
    ContextType current_ctx = get_current_context();

    // If the current context is NONE, enum defs, object defs and function defs are allowed
    // as well as assignments and such, nothing else
    if (current_ctx == CTX_NONE)
    {
        switch(ctx_type)
        {
            case CTX_ENUM:
            case CTX_OBJECT:
            case CTX_FUNCTION: return 1;

            default: return 0;
        }
    }

    // If the current context is a function, object def and enum def is not allowed, everything else is
    if (current_ctx == CTX_FUNCTION)
    {
        switch(ctx_type)
        {
            case CTX_OBJECT:
            case CTX_ENUM: return 0;

            default: return 1;
        }
    }

    // No context is allowed within an enum or object def
    if (current_ctx == CTX_ENUM || current_ctx == CTX_OBJECT)
    {
        return 0;
    }

    // No func def, object def or enum def inside a loop or ifs

    switch (current_ctx)
    {
        case CTX_FOR:
        case CTX_FOREACH:
        case CTX_WHILE:
        case CTX_IF:
        case CTX_ELSE:
        {
            if (ctx_type == CTX_ENUM || ctx_type == CTX_OBJECT || ctx_type == CTX_FUNCTION)
            {
                return 0;
            }
        }

        default: return 1;
    }

    // If we get here, is fine
    return 1;
}


//Check if parent is a loop ---> We could have a for loop and then a skip inside an if, 
//thats allowed 
int current_ctx_is_inside_loop()
{
    int scope_tracker = context.current_scope;
    while (scope_tracker > 1)
    {
        switch (context.ctx_type_stack[scope_tracker])
        {   
            case CTX_NONE: return 0;
            case CTX_FUNCTION: return 0;
            case CTX_FOR: return 1;
            case CTX_FOREACH: return 1;
            case CTX_WHILE: return 1;
            default: scope_tracker--;
        }
    }

    return 0;

}


int search_start_func()
{
    for (int i = 0; i < gb_functionTable->count; i++)
    {
        if (strcmp("start", gb_functionTable->functions[i]->identifier) == 0)
        {
            return i;
        }
    }

    fprintf(stderr, "Entry point: start(), not defined at all or not defined in a global scope. \n");
    exit(1);
}
// Type checking functions

int is_type_str(char *str)
{
    return (strcmp(str,"str") == 0);
}

int is_type_signed_int(char *str) 
{
    return (strcmp(str, "int8") == 0 || strcmp(str, "int16") == 0 || strcmp(str, "int32") == 0);
}

int is_type_unsigned_int(char *str) 
{
    return (strcmp(str, "uint8") == 0 || strcmp(str, "uint16") == 0 || strcmp(str, "uint32") == 0);
}

int is_type_int(char *str) 
{
    return is_type_signed_int(str) || is_type_unsigned_int(str);
}

int is_type_int8(char *str) 
{
    return (strcmp(str, "int8") == 0 || strcmp(str, "uint8") == 0);
}

int is_type_char(char *str) 
{
    return strcmp(str, "char") == 0;
}

int is_type_ptr(char *str)
{
    return strncmp(str, "ptr", 3) == 0 || strncmp(str, "str",3) == 0;
}

int is_type_subscriptable(char *type)
{
    return is_type_array(type) || is_type_ptr(type);
}


int is_type_bool(char *str) 
{
    return strcmp(str, "bool") == 0;
}

int is_type_enum(EnumTable *et, char *type)
{
    for (int i = 0; i < et->count; i++)
    {
        if (strcmp(et->enums[i]->identifier, type) == 0)
        {
            return 1;
        }
    }

    return 0;
}

char *get_int_type(int64_t n) 
{

    if (n >= 0)
    {
        if (n <= UINT8_MAX) return "uint8";
        if (n <= UINT16_MAX) return "uint16";
        if (n <= UINT32_MAX) return "uint32";
    }
    
    else
    {
        // Check signed ranges
        if (n >= INT8_MIN && n <= INT8_MAX) return "int8";
        if (n >= INT16_MIN && n <= INT16_MAX) return "int16";
        if (n >= INT32_MIN && n <= INT32_MAX) return "int32";
    }
    
    
    fprintf(stderr, "Value: %" PRId64 ", is too big to store. Integer overflow.\n", n);
    exit(1);
}

void compare_ret_type(char *func_name, char *declared, char *actual) {
    // Allow exact matches
    if (strcmp(declared, actual) == 0) return;
    
    // Integer type compatibility rules
    if (is_type_int(declared) && is_type_int(actual)) {
        // Get size ranks (1=8bit, 2=16bit, 3=32bit)
        int declared_rank = (strstr(declared, "8") ? 1 : (strstr(declared, "16") ) ? 2 : 3);
        int actual_rank = (strstr(actual, "8") ? 1 : (strstr(actual, "16")) ? 2 : 3);
        
        // Check sign compatibility
        int declared_unsigned = is_type_unsigned_int(declared);
        int actual_unsigned = is_type_unsigned_int(actual);
        
        // Signed <- Unsigned requires explicit cast in most languages
        if (!declared_unsigned && actual_unsigned) {
            fprintf(stderr, "Function %s: Cannot implicitly convert unsigned type %s to signed type %s\n",
                    func_name, actual, declared);
            exit(1);
        }
        
        /*
        // Size check
        if (actual_rank > declared_rank) 
        {
            fprintf(stderr, "Function %s: Possible overflow converting %s to %s\n",
                    func_name, actual, declared);
            exit(1);
        }
        */
        return;
    }
    

    //enum type and uint8 are the same 
    if ( (is_type_enum( gb_enumTable ,declared) && strcmp(actual, "uint8") == 0 )  || 
        ( strcmp(declared, "uint8") == 0 && is_type_enum(gb_enumTable, actual) ))
    {
        return;
    }

    // Char and uint8 are compatible (assuming char is unsigned)
    if ((strcmp(declared, "char") == 0 && strcmp(actual, "uint8") == 0) ||
        (strcmp(actual, "char") == 0 && strcmp(declared, "uint8") == 0)) {
        return;
    }
    
    // Pointer compatibility rules
    if ((strcmp(declared, "ptr char") == 0 && strcmp(actual, "str") == 0) ||
        (strcmp(declared, "str") == 0 && strcmp(actual, "ptr char") == 0)) {
        return;
    }
    
    // Generic pointer compatibility
    if (strncmp(declared, "ptr", 3) == 0 && strcmp(actual, "ptr any") == 0) {
        return;
    }
    
    // No compatible types found
    fprintf(stderr, "Function %s: Return type mismatch. Declared %s, actual %s\n",
            func_name, declared, actual);
    exit(1);
}

void analyze_types_with_reassign_symbol(char *lvalue_type, char *rvalue_type, TokenType reassign_symbol)
{
    
    // TOKEN_ADD_ASSIGN,
    // TOKEN_SUB_ASSIGN,
    // TOKEN_MUL_ASSIGN,
    // TOKEN_DIV_ASSIGN,
    // TOKEN_MOD_ASSIGN,

    switch(reassign_symbol)
    {
        case TOKEN_ASSIGN: return;
        case TOKEN_ADD_ASSIGN:
        case TOKEN_SUB_ASSIGN:
        case TOKEN_DIV_ASSIGN:
        case TOKEN_MUL_ASSIGN:
        case TOKEN_MOD_ASSIGN:
        {
            if (is_type_ptr(lvalue_type) && is_type_int(rvalue_type)) { return; }
            if (is_type_int(lvalue_type) && is_type_int(rvalue_type)) { return; }

            fprintf(stderr, "Can't use the reassing symbol: '%s' for types that are not integers. Got '%s' and '%s' \n", tokenTypeToStr(reassign_symbol) ,lvalue_type, rvalue_type);
            exit(1);
        }

        default:
        {
            fprintf(stderr, "Makes no sense to have reassign symbol: '%s' to analyze types with reassing symbol\n", tokenTypeToStr(reassign_symbol));
            exit(1);
        }
    }


}


void compare_reassign_types(int line_number, char *lvalue_type, char *rvalue_type)
{
    char *declared = lvalue_type;
    char *actual = rvalue_type;
    if (strcmp(declared, actual) == 0) return;
     
    if (is_type_array(declared) != is_type_array(actual))
    {
        fprintf(stderr, "Src: '%s' . Reassignment in L=%i: LValue expects type: '%s' but got: '%s'\n", tracker.current_src_file, line_number ,declared, actual);
        exit(1);
    }
    
    // 24-11 : No need to get base type here, they just gotta be the same
    //char *declared_base = get_base_type(declared);
    //char *actual_base = get_base_type(actual);

    char *declared_base = declared;
    char *actual_base = actual;
   
    if (is_type_int(declared_base) && is_type_int(actual_base)) 
    {
        int declared_rank = (strstr(declared_base, "8") ? 1 : (strstr(declared_base, "16")) ? 2 : 3);
        int actual_rank = (strstr(actual_base, "8") ? 1 : (strstr(actual_base, "16")) ? 2 : 3);
        
        int declared_unsigned = is_type_unsigned_int(declared_base);
        int actual_unsigned = is_type_unsigned_int(actual_base);
        
        if (declared_unsigned && !actual_unsigned) 
        {
            fprintf(stderr, "Src: '%s'. Reassignment in L=%i: LValue expects unsigned: %s, but actual type is signed: %s\n",
                    tracker.current_src_file, line_number, declared, actual);
            exit(1);
        }
        
        return;
    }
    

    //enum type and uint8 are the same 
    if ( (is_type_enum( gb_enumTable ,declared_base) && strcmp(actual_base, "uint8") == 0 )  || 
        ( strcmp(declared_base, "uint8") == 0 && is_type_enum(gb_enumTable, actual_base) ))
    {
        return;
    }

    //chars and uint8 are the same
    if ((strcmp(declared_base, "char") == 0 && strcmp(actual_base, "uint8") == 0) ||
        (strcmp(actual_base, "char") == 0 && strcmp(declared_base, "uint8") == 0)) 
    {
        return;
    }
    
    //ptr char and str are the same
    if ((strcmp(declared_base, "ptr char") == 0 && strcmp(actual_base, "str") == 0) ||
        (strcmp(declared_base, "str") == 0 && strcmp(actual_base, "ptr char") == 0)) 
    {
        return;
    }
    

    if (strncmp(declared_base, "ptr", 3) == 0 && strcmp(actual_base, "ptr any") == 0) {
        return;
    }
    
    fprintf(stderr, "Src: '%s' . Reassignment in L=%i: LValue expects type: '%s', but got: '%s'\n",
            tracker.current_src_file, line_number, declared, actual);
    exit(1);

}

// Get the element type if its an array and check with that
// as well as the size of the array
void compare_types(Symbol *s, char *declared, char *actual) 
{
    char *identifier = s->identifier;
    //printf("Comparing types %s and %s \n", declared, actual);
    // Use the same rules as return type checking
    if (strcmp(declared, actual) == 0) return;

    
    

    char *declared_base = declared;
    char *actual_base = actual;

    int n_elements_declared = is_type_array(declared);
    int n_elements_actual = is_type_array(actual);

    if (n_elements_declared != n_elements_actual)
    {
        fprintf(stderr, "Variable %s: Expects type: %s, but actual type is : %s\n",
                    identifier, declared, actual);
        exit(1);
    }

    // At this point, either both types are array or none is. If they are arrays, get the element type to compare
    if (n_elements_actual)
    {
        declared_base = get_arr_element_type(declared);
        actual_base = get_arr_element_type(actual);
    }

   
    if (is_type_int(declared_base) && is_type_int(actual_base)) 
    {
        int declared_rank = (strstr(declared, "8") ? 1 : (strstr(declared, "16")) ? 2 : 3);
        int actual_rank = (strstr(actual, "8") ? 1 : (strstr(actual, "16")) ? 2 : 3);
        
        int declared_unsigned = is_type_unsigned_int(declared);
        int actual_unsigned = is_type_unsigned_int(actual);
        
        if (declared_unsigned && !actual_unsigned) 
        {
            fprintf(stderr, "Variable %s: Expects unsigned: %s, but actual type is signed: %s\n",
                    identifier, declared, actual);
            exit(1);
        }

        if (declared_rank >= actual_rank) { return; }
    }
    

    //enum type and uint8 are the same 
    if ( (is_type_enum( gb_enumTable ,declared) && strcmp(actual, "uint8") == 0 )  || 
        ( strcmp(declared, "uint8") == 0 && is_type_enum(gb_enumTable, actual) ))
    {
        return;
    }

    //chars and uint8 are the same
    if ((strcmp(declared, "char") == 0 && strcmp(actual, "uint8") == 0) ||
        (strcmp(actual, "char") == 0 && strcmp(declared, "uint8") == 0)) 
    {
        return;
    }
    
    //ptr char and str are the same
    if ((strcmp(declared, "ptr char") == 0 && strcmp(actual, "str") == 0) ||
        (strcmp(declared, "str") == 0 && strcmp(actual, "ptr char") == 0)) 
    {
        return;
    }
    

    if ( (strncmp(declared, "ptr", 3) == 0 && strcmp(actual, "ptr any") == 0) || 
        (strcmp(declared, "ptr any") == 0 && strncmp(actual, "ptr", 3) == 0)) {
        return;
    }
    
    fprintf(stderr, "Variable %s: Type mismatch. Expected %s, got %s\n",
            identifier, declared, actual);
    exit(1);
}

char *resolve_final_ptr_type(const char *ptr_type, int deref_levels)
{
    // Make a copy of the input string to avoid modifying the original
    char *type = malloc(strlen(ptr_type) + 1);
    strcpy(type, ptr_type);

    // Iterate through the string to resolve the final type
    char *current = type;

    for (int i = 0; i < deref_levels; i++)
    {
        //Check if the current token is "str"
        if (strncmp(current, "str", 3) == 0)
        {
            return "char";
        }
        // Check if the current token is "ptr"
        if (strncmp(current, "ptr", 3) != 0)
        {
            fprintf(stderr, "Invalid pointer type: %s\n", type);
            free(type);
            exit(1);
        }

        // Move to the next token
        current += 3; // Skip "ptr"
        while (*current == ' ') current++; // Skip any spaces
    }

    // Return the final type (remaining part of the string)
    return strdup(current);
}


char *short_for_type(char *type)
{
    /* 1. Make a writable copy and strip all whitespace */
    char cleaned[256];                          /* adjust size if you expect very long names */
    remove_whitespace_copy(cleaned, type, sizeof(cleaned));

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


void validate_binary_op(char *op, char *type_left, char *type_right)
{
    //operations between 2 pointers
    if (strncmp(type_left, "ptr", 3) == 0 && strncmp(type_right, "ptr", 3) == 0)
    {
        if (strcmp(op,"==") == 0) { return; }
        if (strcmp(op,"!=") == 0) { return; }
        
        fprintf(stderr, "Trying to perform binary operation: %s, between type: %s, and type: %s. Undefined. \n", op, type_left, type_right);
        exit(1);
    }

    //pointer arithmetic (ptr + uint) allowed
    if ( (strncmp(type_left, "ptr", 3) == 0 && is_type_unsigned_int(type_right)) ||
         (strncmp(type_right, "ptr", 3) == 0 && is_type_unsigned_int(type_left)))
    {
        return;
    }
    
    //equal types, we are good
    if (strcmp(type_left, type_right) == 0) { return; }

    //numbers, binary operations allowed between any of these types, any combination. programmer's responsibility
    if (is_type_int(type_left) && is_type_int(type_right)) { return; }

    // Operations between char and any integer type are allowed
    if ((is_type_char(type_left) && is_type_int(type_right)) ||
        (is_type_char(type_right) && is_type_int(type_left))) {
        return;
    }

    // Booleans: operations between bool and integers are allowed
    if ((is_type_bool(type_left) || is_type_bool(type_right)) &&
        (is_type_int(type_left) || is_type_int(type_right)))
    {
        return;
    }

    // integers and enums 
    if ((is_type_enum(gb_enumTable, type_left) || is_type_enum(gb_enumTable,type_right) )  && 
        (is_type_int(type_left) || is_type_int(type_right)))
    {
        return;
    }

    if ( (strcmp(type_left, "str") == 0 || strcmp(type_right, "str") == 0) &&
            (is_type_unsigned_int(type_left) || is_type_unsigned_int(type_right)) )
    {
        return;
    }




    fprintf(stderr, "Trying to perform binary operation: %s, between type: %s, and type: %s. Undefined. \n", op, type_left, type_right);
    exit(1);
}


char *get_function_signature(Function *f)   
{
    char *func_signature = malloc(sizeof(char) * 200);
    memset(func_signature, 0, 200); // <-- Zero the buffer

    char **param_types = get_paramTypes_from_Params(f->params, f->param_count);

    sprintf(func_signature, "_%s", f->identifier);

    for (int i = 0; i < f->param_count; i++)
    {
        char buffer[32] = {0};
        sprintf(buffer, "_%s", short_for_type(param_types[i]));
        strcat(func_signature, buffer);
    }

    func_signature = realloc(func_signature, strlen(func_signature) + 1);
    return func_signature;
}

char *get_function_ret_signature(Function *f)
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



void print_str_table()
{
    printf("============================== STRING TABLE =========================== \n");
    for (int i = 0; i < stringTable->count; i++)
    {
        printf("%i: (label = %s, str = %s)\n", i, stringTable->labels[i], stringTable->saved_strings[i]);
    }
    printf("============================== END STRING TABLE =========================== \n");

}

void init_string_table(int size)
{
    stringTable = malloc(sizeof(StringTable));

    stringTable->saved_strings = malloc(size * sizeof(char *));
    stringTable->labels = malloc(size * sizeof(char *));
    stringTable->count = 0;
    stringTable->size = size;
}

void add_str_to_table(char *str)
{
    // Reallocate if needed
    if (stringTable->count + 1 >= stringTable->size)
    {
        stringTable = realloc(stringTable, 2 * stringTable->size);
        stringTable->size = 2 * stringTable->size;
    }

    stringTable->saved_strings[stringTable->count] = str;

    char *label = malloc(sizeof(char) * 32);
    snprintf(label, sizeof(label), "Lstr%i", stringTable->count);

    stringTable->labels[stringTable->count] = label; 

    stringTable->count++;
}

char *get_last_str_label()
{
    if (stringTable->count - 1 < 0)
    {
        fprintf(stderr, "Cannot get last str label because there are no labels\n");
        exit(1);
    }
    return stringTable->labels[stringTable->count - 1];
}

void init_gb_symbol_table(int size)
{
    gb_symbolTable = malloc( sizeof (SymbolTable));
    gb_symbolTable->symbols = malloc(size * sizeof(Symbol *));
    gb_symbolTable->size = size;
    gb_symbolTable->count = 0;
    gb_symbolTable->parentST = NULL;
    gb_symbolTable->scope = 0;
}

void init_gb_function_table(int size)
{
    gb_functionTable = malloc(sizeof (FunctionTable ));
    gb_functionTable->functions = malloc(size * sizeof (Function *));
    gb_functionTable->size = size;
    gb_functionTable->count = 0;
    gb_functionTable->scope = 0;
    gb_functionTable->parentFT = NULL;
}

void init_gb_object_table(int size)
{
    gb_objectTable = malloc(sizeof (ObjectTable ));
    gb_objectTable->objects = malloc(size * sizeof(Object *));
    gb_objectTable->size = size;
    gb_objectTable->count = 0;
}

void init_gb_enum_table(int size)
{
    gb_enumTable = malloc(sizeof(EnumTable));
    gb_enumTable->enums = malloc(size * sizeof(Enum *));
    gb_enumTable->size = size;
    gb_enumTable->count = 0;
}

SymbolTable *init_object_lc_symbol_table(SymbolTable *parentST, int size)
{

    SymbolTable *st = malloc(sizeof(SymbolTable));
    st->symbols = malloc(size * sizeof(Symbol *));
    st->kind = KIND_OBJECT_ST;
    st->count = 0;
    st->padding = 0;
    st->size = size;
    st->parentST = parentST;
    st->scope = parentST->scope + 1;
    return st;
}


SymbolTable *init_lc_symbol_table(SymbolTable *parentST, int size, SCRIPT_MODE script_mode)
{
    if (script_mode == SCRIPT_KERNEL) { return gb_symbolTable; }

    SymbolTable *st = malloc(sizeof(SymbolTable));
    st->symbols = malloc(size * sizeof(Symbol *));
    st->kind = KIND_FUNCTION_ST;
    st->count = 0;
    st->padding = 0;
    st->size = size;
    st->parentST = parentST;
    st->scope = parentST->scope + 1;
    return st;
}

FunctionTable *init_lc_function_table(FunctionTable *parentFT, int size)
{
    FunctionTable *ft = malloc(sizeof(FunctionTable));
    ft->functions = malloc(size * sizeof(Function *));
    ft->count = 0;
    ft->size = size;
    ft->scope = parentFT->scope + 1;
    ft->parentFT = parentFT;
    return ft;
}


void resize_symbol_table(SymbolTable *st, int new_size)
{
    if (new_size <= st->size)
    {
        fprintf(stderr,"New st size (%d) is smaller or equal than previous (%d) \n", new_size, st->size);
        exit(1);
    }
    st->symbols = realloc(st->symbols,new_size * sizeof(Symbol *));
    st->size = new_size;
}

void resize_function_table(FunctionTable *ft, int new_size)
{
    if (new_size <= ft->size)
    {
        fprintf(stderr,"New ft size (%d) is smaller or equal than previous (%d) \n", new_size, ft->size);
        exit(1);
    }
    ft->functions = realloc(ft->functions,new_size * sizeof(Function *));
    ft->size = new_size;
}

void resize_object_table(ObjectTable *ot, int new_size)
{
    if (new_size <= ot->size)
    {
        fprintf(stderr,"New ot size (%d) is smaller or equal than previous (%d) \n", new_size, ot->size);
        exit(1);
    }
    ot->objects = realloc(ot->objects,new_size * sizeof(Object *));
    ot->size = new_size;
}

void resize_enum_table(EnumTable *et, int new_size)
{
    if (new_size <= et->size)
    {
        fprintf(stderr, "New et size (%d) is smaller or equal than previous (%d) \n", new_size, et->size);
        exit(1);
    }

    et->enums = realloc(et->enums, new_size * sizeof(Enum *));
    et->size = new_size;
}

char * ctxToString(ContextType ctx_type)
{
    switch (ctx_type) 
    {
        case CTX_WHILE: return "WHILE";
        case CTX_FUNCTION: return "FUNCTION";
        case CTX_FOR: return "FOR";
        case CTX_FOREACH: return "FOREACH";
        case CTX_IF: return "IF";
        case CTX_ELSE: return "ELSE";
        case CTX_NONE: return "NONE";
        case CTX_ENUM: return "ENUM";
        case CTX_OBJECT: return "OBJECT";
    }
}


int assign_func_id()
{
    return func_id++;
}

int do_symbols_clash(Symbol *s1, Symbol *s2)
{
    // If different name, we good
    if (strcmp(s1->identifier, s2->identifier) != 0) { return 0; }

    // If different scope, we good. 30/11 ARE WE THOUGH
    // If one of them is global then we are not
    if (s1->scope != s2->scope) { return 0; }


    //If different context, we good 
    if (s1->declaration_ctx != s2->declaration_ctx) { return 0; }
    //If different block_id, we good
    if (s1->ctx_block_id != s2->ctx_block_id) { return 0; }

    //Otherwise, these two symbols have the same name, defined in the same context and in the same block
    // => Redefinition 
    return 1;
}


// Check there is no duplicates within the current st AND THE GLOBAL ST
void add_symbol(SymbolTable *st, Symbol *s)
{

    //printf("Trying to add symbol: %s, st->count = %i \n", s->identifier, st->count);

    // Dont allow duplicates within the global st
    for (int i = 0; i < gb_symbolTable->count; i++)
    {
        if (strcmp(s->identifier, gb_symbolTable->symbols[i]->identifier) == 0)
        {
            fprintf(stderr, "Redefinition of global variable: %s, declared in context: %s \n", s->identifier, ctxToString(s->declaration_ctx));
            exit(1);
        }
    }

    //Dont allow duplicates within the supplied st
    for (int i = 0; i < st->count; i++)
    {
        if (do_symbols_clash(st->symbols[i], s) == 1)
        {
            fprintf(stderr, "Redefinition of variable: %s, declared in context: %s \n", s->identifier, ctxToString(s->declaration_ctx));
            exit(1);
        }
    }

    //Resize if needed
    if (st->count + 1 >= st->size) { resize_symbol_table(st, 2 * st->size); }

    // Finally add the size of the symbol 
    s->size = size_of_type(s->type);
    //printf("Size of %s = %i\n", s->identifier, s->size);

    // If it's the smallest sized symbol, add at the end, otherwise it will be bigger than some element 
    // and index of s will be changed within the for loop
    int index_of_s = st->count;

    // If the symbol table belongs to a function, then we sort to reduce padding. 
    // Don't sort if it belongs to an object
    if (st->kind == KIND_FUNCTION_ST)
    {
        //Order the table in correct order, big sizes first
        for (int i = 0; i < st->count; i++)
        {
            if (s->size >= st->symbols[i]->size)
            {
                index_of_s = i;
                // Add one to all the consecutive symbols' index
                // and shift them one down
                for (int j = st->count; j > i; j--)
                {
                    st->symbols[j] = st->symbols[j - 1];
                    st->symbols[j]->index++;

                }

                break;
            }
        }
    }

    
    // Add the symbol in its correct spot within the table
    st->count++;
    s->index = index_of_s;
    st->symbols[s->index] = s;


    
    // Finally, recompute the offsets for the symbols with the reorganized table
    // only need to do so for the new inserted symbol and after
    for (int i = s->index; i < st->count; i++)
    {
        st->symbols[i]->offset =  (i == 0) ? 0 : st->symbols[i - 1]->offset + st->symbols[i-1]->size;
        // Align to an offset that is multiple of the variable size
        //st->symbols[i]->offset = align_to(st->symbols[i]->offset, st->symbols[i]->size);
    }
    

    //print_symbolTable(st);

    // Add the size of the symbol to the total weight of the table
    st->weight += s->size;


    // Keep track of the padding 
    st->padding = 0;
    for (int i = 1; i < st->count; i++)
    {
        st->padding += st->symbols[i]->offset - (st->symbols[i - 1]->offset + st->symbols[i - 1]->size);
    }



    //printf("WEIGHT OF THE TABLE = %i, PADDING = %i\n", st->weight, st->padding);

}

int assign_symbol_scope(SCRIPT_MODE script_mode)
{

    //All variables are global in kernel mode
    if (script_mode == SCRIPT_KERNEL) { return 0; }
    //In user mode, we have locals 
    return context.current_scope;
}

int assign_ctx_block_id(SCRIPT_MODE script_mode)
{
    //Dont know what to do here yet
    if (script_mode == SCRIPT_KERNEL) { return -1; }

    switch (get_current_context()) 
    {
        case CTX_NONE: return 0;
        case CTX_FUNCTION: return context.function_block_id;
        case CTX_FOR: return context.for_block_id;
        case CTX_FOREACH: return context.foreach_block_id;
        case CTX_WHILE: return context.while_block_id;
        case CTX_IF: return context.if_block_id;
        case CTX_ELSE: return context.else_block_id; 
    }
}

int do_functions_clash(Function *f1, Function *f2)
{
    //Different name we good
    if (strcmp(f1->identifier, f2->identifier) != 0) { return 0; }
    //Different scope, we good 
    if (f1->scope != f2->scope) { return 0; }
    //Different parameters count, we good 
    if (f1->param_count != f2->param_count) { return 0; }
    //Check the parameters list 
    int params_matching = 0;
    for (int i = 0; i < f1->param_count; i++)
    {
        //Different parameters supertype, we good, continue
        if (f1->params[i]->supertype != f2->params[i]->supertype) { continue; }
        
        //Same parameters supertype and same number of params
        //Check inmmediate first 
        if (f1->params[i]->supertype == PARAM_IMMEDIATE)
        {
            if (strcmp(f1->params[i]->immediateParam.type, f2->params[i]->immediateParam.type) != 0) { continue; }
            params_matching++;
        }

        //Check callback now 
        if (f1->params[i]->supertype == PARAM_FUNCTION)
        {
            //Different callback params count 
            if (f1->params[i]->functionParam.params_count != f2->params[i]->functionParam.params_count) { continue; }

            int callback_params_matching = 0;
            //Same count. Check types 
            for (int k = 0; k < f1->params[i]->functionParam.params_count; k ++)
            {
                //Different types, we good 
                if (strcmp(f1->params[i]->functionParam.params_type[k], f2->params[i]->functionParam.params_type[k]) != 0) { continue; }
                //Same type
                callback_params_matching++;
            }
            
            if (callback_params_matching == f1->params[i]->functionParam.params_count) { params_matching++; }
        }
    }
    
    if (params_matching != f1->param_count) { return 0; }

    //Same name, same scope, same list of parameter types (and if callback, same param_count and list of types)
    return 1;


}

void add_function(FunctionTable *ft, Function *f)
{
    //Check for duplicate functions 
    for (int i = 0; i < ft->count; i++)
    {
        if (do_functions_clash(ft->functions[i], f) == 1)
        {
            fprintf(stderr, "Redefinition of function : %s, with parameters: \n", f->identifier);
            for (int i = 0; i < f->param_count; i++)
            {
                if (f->params[i]->supertype == PARAM_IMMEDIATE)
                {
                    printf("\'%s\'", f->params[i]->immediateParam.type);
                }
                
                else if (f->params[i]->supertype == PARAM_FUNCTION)
                {
                    printf("\'%s\'(", f->params[i]->functionParam.type);
                    for (int k = 0; k < f->params[i]->functionParam.params_count; k++)
                    {
                        printf("\'%s\'", f->params[i]->functionParam.params_type[k]);

                        if (k < f->params[i]->functionParam.params_count - 1)
                        {
                            printf(",");
                        }
                    }
                    printf(")");
                }

                if (i < f->param_count - 1)
                {
                    printf(",");
                }
            }
            printf("\n");
        }
    }

    if (ft->count + 1 >= ft->size) { resize_function_table(ft, 2 * ft->size); }

    ft->functions[ft->count++] = f;
}

void add_object(ObjectTable *ot, Object *o)
{
    //check for duplicate objects
    for (int i = 0; i < ot->count; i++)
    {
        if (strcmp(ot->objects[i]->identifier,o->identifier) == 0)
        {
            fprintf(stderr, "Redefinition of object: %s \n", o->identifier);
            exit(1);
        }
    }

    //add
    if (ot->count + 1 >= ot->size) { resize_object_table(ot, 2 * ot->size); }

    //printf("ADDED OBJECT: %s, TO THE TABLE \n", o->identifier);
    ot->objects[ot->count++] = o;
}

void add_enum(EnumTable *et, Enum *e)
{
    //check for duplicate enums 
    for (int i = 0; i < et->count; i++)
    {
        if (strcmp(et->enums[i]->identifier, e->identifier) == 0)
        {
            fprintf(stderr, "Redefinition of enum: %s \n", e->identifier);
            exit(1);
        }
    }

    //Add
    if (et->count + 1 >= et->size) { resize_enum_table(et, 2 * et->size); }
    et->enums[et->count++] = e;
}


Symbol *check_strict_identifier(SymbolTable *st, int scope, char *identifier)
{
    // printf("Trying to find identifier: %s, in current_scope: %i \n", identifier, scope);

    int checking_scope = scope;
    for (int i = 0; i < st->count; i++)
    {
        //If not the same scope, continue (so retrieve the match in the closest scope)
        if (st->symbols[i]->scope != checking_scope) { continue; }
        
        //If in the same scope and same name, retrieve it
        if (strcmp(st->symbols[i]->identifier, identifier) == 0)
        {
            return st->symbols[i];
        }
    }    
    return NULL;
}

Symbol *check_identifier(SymbolTable *st, int scope, char *identifier)
{
    // printf("Trying to find identifier: %s, in current_scope: %i \n", identifier, scope);
    while (st != NULL)
    {
        int checking_scope = scope;
        while (checking_scope >= 0)
        {
            for (int i = 0; i < st->count; i++)
            {
                //If not the same scope, continue (so retrieve the match in the closest scope)
                if (st->symbols[i]->scope != checking_scope) { continue; }
                
                //If in the same scope and same name, retrieve it
                if (strcmp(st->symbols[i]->identifier, identifier) == 0)
                {
                    return st->symbols[i];
                }
            }

            checking_scope--;
        }        
        st = st->parentST; // Move to the parent scope
    }
    
    return NULL;
}

//PRIORITY: BIGGEST SCOPE FIRST
Symbol* lookup_identifier(SymbolTable *st, int scope, char *identifier)
{
    // printf("Trying to find identifier: %s, in current_scope: %i \n", identifier, scope);
    while (st != NULL)
    {
        int checking_scope = scope;
        while (checking_scope >= 0)
        {
            for (int i = 0; i < st->count; i++)
            {
                //If not the same scope, continue (so retrieve the match in the closest scope)
                if (st->symbols[i]->scope != checking_scope) { continue; }
                
                //If in the same scope and same name, retrieve it
                if (strcmp(st->symbols[i]->identifier, identifier) == 0)
                {
                    return st->symbols[i];
                }
            }

            checking_scope--;
        }        
        st = st->parentST; // Move to the parent table
    }

    fprintf(stderr, "Variable : %s, not found in current scope nor in any parent scope. \n", identifier);
    exit(1);
}

Symbol *lookup_field(Object *object, char *field_id)
{
    Object *curr_object = object;
    while (curr_object != NULL)
    {
        for (int i = 0; i < curr_object->local_symbols->count; i++)
        {
            if (strcmp(curr_object->local_symbols->symbols[i]->identifier, field_id) == 0)
            {
                return curr_object->local_symbols->symbols[i];
            }
        }
        curr_object = curr_object->parent; // Move to the parent scope
    }

    fprintf(stderr, "Field : %s, not found in object definition nor in any parent object. \n", field_id);
    exit(1);
}


 Symbol *get_ParamAtIndex(SymbolTable *st, int index)
 {
    for (int i = 0; i < st->count; i++)
    {
        if (st->symbols[i]->param_index == index) { return st->symbols[i]; }
    }

    fprintf(stderr, "No parameter at index: %s was found \n", index);
    exit(1);
 }

char **get_paramTypes_from_Params(Param **params, int param_count)
{
    char **paramTypes = malloc(100 * sizeof(char *));
    for (int i = 0; i < param_count; i++)
    {
        if (params[i]->supertype == PARAM_IMMEDIATE)
        {
            paramTypes[i] = params[i]->immediateParam.type;
            continue;
        }

        paramTypes[i] = "callback";
    }

    return paramTypes;
}

int get_type_ptr_depth(char *type)
{
    int depth = 0;
    const char *s = type;

    while (1) {
        // Skip whitespace
        while (*s == ' ') s++;

        // Check if the next word is "ptr"
        if (strncmp(s, "ptr", 3) == 0 && (s[3] == ' ' || s[3] == '\0')) {
            depth++;
            s += 3; // Move past "ptr"
        } else {
            break; // Not "ptr", stop counting
        }
    }

    return depth;
}

int is_sub_integer(char *candidateType, char *paramType)
{
    if (strcmp(paramType, "int8") == 0 && 
    (strcmp(candidateType, "int32") == 0 || strcmp(candidateType, "int16") == 0)) { return 1; }

    if (strcmp(paramType, "int16") == 0 && strcmp(candidateType, "int32") == 0) { return 1; }
    
    if (strcmp(paramType, "uint8") == 0 && 
    (strcmp(candidateType, "uint32") == 0 || strcmp(candidateType, "uint16") == 0)) { return 1; }

    if (strcmp(paramType, "uint16") == 0 && strcmp(candidateType, "uint32") == 0) { return 1; }
 

    return 0;
}

int is_acceptable_param_type(char *candidateType, char *paramType)
{     

    //Equal case
    if (strcmp(candidateType, paramType) == 0) { return 1; }
    
    //Different integers? if (candidateType.integer < paramType) -> return 1
    if (is_sub_integer(candidateType, paramType)) { return 1; }

    //ptr<type> and ptr<any>
    if (strcmp(candidateType, "ptr any") == 0 && 
        (strncmp(paramType, "ptr",3) == 0 || strcmp(paramType, "str") == 0))
    {
        return 1;
    }

    //If different pointer depths
    if (get_type_ptr_depth(candidateType) != get_type_ptr_depth(paramType)) { return 0; }
    
    //At this point, both have the same depth
    //Get the base type of both

    // 24/11: No need to get base type
    //char *paramBaseType = get_base_type(paramType);
    //char *candidateBaseType = get_base_type(candidateType);


    char *paramBaseType = paramType;
    char *candidateBaseType = candidateType;

    //Family types (paramType inherits from candidate)
    Object *child = check_object(gb_objectTable, paramBaseType);
    while (child != NULL)
    {
        if (strcmp(child->identifier, candidateBaseType) == 0)
        {
            //printf("paramType: %s, is acceptable for expected type: %s", )
            return 1;
        }
        child = child->parent;
    }
    
    //Else is not valid
    return 0;

}

Function *check_function(FunctionTable *ft, int scope, char *identifier, char **param_types, int param_count)
{
    //printf("Trying to check function : %s \n", identifier);
    while (ft != NULL)
    {
        //print_functionTable(ft);
        for (int i = 0; i < ft->count; i++)
        {
            Function *candidate = ft->functions[i];
            //Find function with the same name within the allowed scope
            if (strcmp(candidate->identifier, identifier) == 0
                && scope >= candidate->scope)
            {
                //Find function with the same number of parameters
                if (candidate->param_count != param_count) { continue; }

                int match = 1;
                for (int k = 0; k < param_count; k++)
                {
                    char *candidateType = (candidate->params[k]->supertype == PARAM_FUNCTION) ? "callback" : 
                                            candidate->params[k]->immediateParam.type;
                    if (!is_acceptable_param_type(candidateType, param_types[k]))
                    {
                        match = 0;
                        break;
                    }
                }

                if (match) 
                { 
                    //printf("Found a match.\n"); 
                    return candidate; 
                }

            }
        }
        ft = ft->parentFT; // Move to the parent scope
    }

    return NULL;
}


Function *lookup_function(FunctionTable *ft, int scope, char *identifier, char **param_types, int param_count)
{
    while (ft != NULL)
    {
        //printf("Trying to find function f:%s, at scope = %i\n", identifier, scope);
        for (int i = 0; i < ft->count; i++)
        {
            Function *candidate = ft->functions[i];
            //printf("Candidate name = %s, scope = %i\n", candidate->identifier, candidate->scope);
            //Find function with the same name within the allowed scope
            if (strcmp(candidate->identifier, identifier) == 0
                && scope >= candidate->scope)
            {
                //Find function with the same number of parameters
                if (candidate->param_count != param_count) { continue; }

                int match = 1;
                for (int k = 0; k < param_count; k++)
                {
                    char *candidateType = (candidate->params[k]->supertype == PARAM_FUNCTION) ? "callback" : 
                                            candidate->params[k]->immediateParam.type; 
                    if (!is_acceptable_param_type(candidateType, param_types[k]))
                    {
                        match = 0;
                        break;
                    }
                }

                if (match) { return candidate; }

            }
        }
        ft = ft->parentFT; // Move to the parent scope
    }

    fprintf(stderr, "Trying to fetch function: %s, with parameters of type: \n", identifier);
    for (int i = 0; i < param_count; i++)
    {
       
        printf("\'%s\'", param_types[i]);
        if (i < param_count - 1)
        {
            printf(",");
        }
    }
    printf("\nSuch function not found in current scope nor in any parent scope.\n");
    exit(1);
}

Object *check_object(ObjectTable *ot, char *identifier)
{
    for (int i = 0; i < ot->count; i++)
    {
        if (strcmp(ot->objects[i]->identifier, identifier) == 0)
        {
            return ot->objects[i];
        }
    }

    return NULL;
}

Object *lookup_object(ObjectTable *ot, char *identifier)
{
    //resolvePtrType()
    for (int i = 0; i < ot->count; i++)
    {
        if (strcmp(ot->objects[i]->identifier, identifier) == 0)
        {
            return ot->objects[i];
        }
    }

    fprintf(stderr, "Object : %s, not found in global scope. \n", identifier);
    exit(1);
}

int align_to(int offset, int alignment)
{
    return (offset + (alignment - 1)) & ~(alignment - 1);
}
int size_of_type(char *type)
{

    int n_elements = is_type_array(type);

    char *type_to_eval;
    if (n_elements >= 1) { type_to_eval = get_arr_element_type(type); }
    else { type_to_eval = type; }
    //char *type_to_eval = get_base_type(type);

    
    //Builtin type
    if (strcmp(type_to_eval, "int32") == 0) { return n_elements? 4 * n_elements : 4; }
    if (strcmp(type_to_eval, "int16") == 0) {return n_elements? 2 * n_elements : 2;}
    if (strcmp(type_to_eval, "int8") == 0) {return n_elements? 1 * n_elements : 1;}
    if (strcmp(type_to_eval, "uint32") == 0) { return n_elements? 4 * n_elements : 4; }
    if (strcmp(type_to_eval, "uint16") == 0) {return n_elements? 2 * n_elements : 2;}
    if (strcmp(type_to_eval, "uint8") == 0) {return n_elements? 1 * n_elements : 1;}
    if (strcmp(type_to_eval, "char") == 0) { return n_elements? 1 * n_elements : 1; }
    if (strcmp(type_to_eval, "bool") == 0) { return n_elements? 1 * n_elements : 1; }
    if (strcmp(type_to_eval, "str") == 0) { return n_elements? 8 * n_elements : 8; }     //Pointer to another location in memory
    if (strcmp(type_to_eval, "any") == 0) { return n_elements? 8 * n_elements : 8; } //Used in ptr<any>
    if (strcmp(type_to_eval, "callback") == 0) { return n_elements? 8 * n_elements : 8; }
    //If none of this, may be a pointer to some builtin data
    //If the first 3 chars of type is "ptr", then is 8 bytes. No matter the depth
    if (strncmp(type_to_eval, "ptr", 3) == 0) { return n_elements? 8 * n_elements : 8; }
    

    if (is_type_enum(gb_enumTable, type)) { return 1; }

    //User defined type
    else
    {
        //Calculate the size of all the fields of the object + fields from the parents
        Object *o = lookup_object(gb_objectTable, type_to_eval);
        int size = 0;
        Object *curr_object = o;
        while (curr_object != NULL)
        {
            for (int i = 0; i < curr_object->local_symbols->count; i++)
            {
                size += size_of_type(curr_object->local_symbols->symbols[i]->type);
            }
            curr_object = curr_object->parent;
        }
        
        return n_elements? size * n_elements : size;
    }
}

// Returns the final pointed type.
// ptr ptr type ----> type
char *get_final_pointed_type(char *src_type)
{
    char *copy = strdup(src_type);  // Work on a copy safely

    char *token = strtok(copy, " ");  
    char *last = NULL;

    // Iterate tokens: ["ptr", "ptr", "uint8"]
    while (token != NULL)
    {
        last = token;
        token = strtok(NULL, " ");
    }

    if (last == NULL)
    {
        fprintf(stderr, "Malformed type: '%s'\n", src_type);
        exit(1);
    }

    // Return final token (base type)
    return strdup(last);
}

// Returns the element type for an array
// type [size] --> type
char *get_arr_element_type(char *src_type)
{
    if (strcmp(src_type, "str") == 0)
    {
        return "char";
    }

    if (!is_type_array(src_type))
    {
        fprintf(stderr, "In 'get_arr_element_size()'. Type '%s' is not an array type.\n", src_type);
        exit(1);
    }

    size_t len = strlen(src_type);

    // Find the start of the last "["
    int bracket_start = -1;
    for (int i = len - 1; i >= 0; i--)
    {
        if (src_type[i] == '[')
        {
            bracket_start = i;
            break;
        }
    }

    if (bracket_start == -1)
    {
        fprintf(stderr, "In 'get_arr_element_size()'. Malformed array type '%s'\n", src_type);
        exit(1);
    }

    // Trim the trailing space before '['
    int end = bracket_start - 1;
    while (end >= 0 && (src_type[end] == ' ' || src_type[end] == '\t'))
    {
        end--;
    }

    if (end < 0)
    {
        fprintf(stderr, "In 'get_arr_element_size()'.Malformed type '%s'\n", src_type);
        exit(1);
    }

    // Now copy everything up to end (inclusive)
    char *result = malloc(end + 2); // +1 for '\0'
    if (!result)
    {
        fprintf(stderr, "In 'get_arr_element_size()'. Out of memory.\n");
        exit(1);
    }

    strncpy(result, src_type, end + 1);
    result[end + 1] = '\0';

    return result;
}


char *get_pointed_type(char *pointed_type, int deref_levels)
{
    if (strcmp(pointed_type, "ptr any") == 0)
    {
        fprintf(stderr, "Type: 'any', cannot be used as a standalone type, only allowed as pointed type: 'ptr<any>'");
        exit(1);
    }

    if (!is_type_ptr(pointed_type))
    {
        fprintf(stderr, "Can't get the pointed type for a type that is not a pointer to start with: '%s'\n", pointed_type);
        exit(1);
    }

    const char* ptr = pointed_type;
    int space_count = 0;

    while (*ptr != '\0' && space_count < deref_levels) 
    {
        if (*ptr == ' ') 
        {
            space_count++;
        }
        ptr++;
    }

    // If we found X spaces, return the remaining string
    if (space_count == deref_levels) 
    {
        return (char*)ptr; // Skip the last space
    } 
    else 
    {
        return NULL; // Fewer than X spaces found
    }
}


char *ensure_type_exists(char *type)
{

     //Check built-in types
    for (int i = 0; i < sizeof(types)/ sizeof(types[0]); i++)
    {
        if (strcmp(type, types[i]) == 0)
        {
            if (strcmp(type, types[i]) == 0)
            {
                return type;
            }
        }
    }

    //Check enums 
    for (int i = 0; i < gb_enumTable->count; i++)
    {
        if (strcmp(gb_enumTable->enums[i]->identifier, type) == 0)
        {
            return type;
        }
    }


    if (is_type_array(type))
    {
        char *element_type = get_arr_element_type(type);
        ensure_type_exists(element_type);

        return type;
    }

    if (is_type_ptr(type))
    {
        char *pointed_type = get_final_pointed_type(type);
        ensure_type_exists(pointed_type);

        return type;
    }

   

    //Check user defined types
    for (int i = 0; i < gb_objectTable->count; i++)
    {
        if (strcmp(type, gb_objectTable->objects[i]->identifier) == 0)
        {
            return type;
        }
    }


    //If none
    fprintf(stderr, "Type : %s, not defined. \n", type);
    exit(1);
}


//Populate the tables and analyze whether expressions are valid
Value analyze(ASTNode *program, SymbolTable *current_st, FunctionTable *current_ft, SCRIPT_MODE script_mode)
{
    if (program == NULL || program->node_type == NODE_NULL) { Value v = {.type = "void"}; return v; }
    //printf("Analyzing node with type: %s \n", astTypeToStr(program));
    
    switch (program->node_type)
    {

        case NODE_USE_DIRECTIVE:
        {
            Value r = {.type = "void"};
            r = analyze(program->use_node.program, current_st, current_ft, script_mode);
            return r;
        }
        case NODE_BLOCK:
        {
            //Default init value if block is empty
            Value r = {.type = "void"};
            for (int i = 0; i < program->block_node.statement_count; i++)
            {
               r = analyze(program->block_node.statements[i], current_st, current_ft, script_mode);
            }

            return r;
        }

        case NODE_NULL:
        {
            Value r = {.type = "void"};
            return r;
        }
        
        case NODE_EXTERN_FUNC_DEF:
        {
             //Create a function. Create its local symbol table and local function table
            Function *f = malloc(sizeof(Function));
            f->unique_id = assign_func_id();

            f->rt_type = program->extern_func_def_node.return_type;
            f->identifier = program->extern_func_def_node.func_name;
            //printf("Created a new function with name: %s and rt: %s \n", f->identifier, f->rt_type);
            f->index = current_ft->count;
            f->param_count = program->extern_func_def_node.params_count;
            f->params = program->extern_func_def_node.params;
            //f->scope = (current_ft == gb_functionTable) ? 0 : 1;
            f->scope = current_ft->scope;
            f->local_symbols = init_lc_symbol_table(current_st, program->extern_func_def_node.params_count + 1, script_mode);        // + 1 just in case
            f->local_functions = init_lc_function_table(current_ft,3);
            add_function(current_ft, f);
            //save the function unique id 
            program->extern_func_def_node.fn_unique_id = f->unique_id;


            //Increment scope, we are inside the function, push the context 
            push_context(CTX_FUNCTION);

            //First populate the function's local symbol table with the parameters and possible functions
            for (int i = 0; i < program->extern_func_def_node.params_count; i++)
            {
                //If it's immediate
                if (program->extern_func_def_node.params[i]->supertype == PARAM_IMMEDIATE)
                {
                    Symbol *s = malloc(sizeof(Symbol));
                    s->identifier = program->extern_func_def_node.params[i]->immediateParam.param_name;
                    //printf("here!\n");

                    s->type = ensure_type_exists(program->extern_func_def_node.params[i]->immediateParam.type);

                    s->scope = assign_symbol_scope(script_mode);
                    s->index = i;
                    s->param_index = i;


                    s->declaration_ctx = get_current_context();
                    s->ctx_block_id = assign_ctx_block_id(script_mode);
                    s->fn_unique_id = -2;
                    add_symbol(f->local_symbols, s);
                    continue;
                }

                //Else assume it's a function reference
                Symbol *s = malloc(sizeof(Symbol));
                s->identifier = program->extern_func_def_node.params[i]->functionParam.identifier;
                s->type = "callback";
                s->scope = assign_symbol_scope(script_mode);
                s->index = i;
                s->param_index = i;

                s->declaration_ctx = get_current_context();
                s->ctx_block_id = assign_ctx_block_id(script_mode);
                s->fn_unique_id = -1;
                s->fn_param_types = program->extern_func_def_node.params[i]->functionParam.params_type;
                s->fn_param_count = program->extern_func_def_node.params[i]->functionParam.params_count;
                s->fn_return_type = program->extern_func_def_node.params[i]->functionParam.return_type;
                add_symbol(f->local_symbols, s);
            }
            

            // Add the signature
            f->signature = get_function_signature(f);
            // Add the return signature
            f->ret_signature = get_function_ret_signature(f);

            //We are outside the function, pop the context and scope 
            pop_context();



            //Statements that return nothing
            Value r = {.type = "void"};
            return r;
        }

        case NODE_FUNC_DEF:
        {
            //Create a function. Create its local symbol table and local function table
            Function *f = malloc(sizeof(Function));
            f->unique_id = assign_func_id();

            f->rt_type = program->funcdef_node.return_type;
            f->identifier = program->funcdef_node.func_name;
            //printf("Created a new function with name: %s and rt: %s \n", f->identifier, f->rt_type);
            f->index = current_ft->count;
            f->param_count = program->funcdef_node.params_count;
            f->params = program->funcdef_node.params;
            //f->scope = (current_ft == gb_functionTable) ? 0 : 1;
            f->scope = current_ft->scope;
            f->local_symbols = init_lc_symbol_table(gb_symbolTable, program->funcdef_node.params_count + 1, script_mode);        // + 1 just in case
            f->local_functions = init_lc_function_table(current_ft,3);
            add_function(current_ft, f);
            //save the function unique id 
            program->funcdef_node.fn_unique_id = f->unique_id;

            //Increment scope, we are inside the function, push the context 
            push_context(CTX_FUNCTION);


            //printf("-------- CHECKING THE PARAMETERS IN F = %s \n", f->identifier);
            //First populate the function's local symbol table with the parameters and possible functions
            for (int i = 0; i < program->funcdef_node.params_count; i++)
            {
                //If it's immediate
                if (program->funcdef_node.params[i]->supertype == PARAM_IMMEDIATE)
                {
                    Symbol *s = malloc(sizeof(Symbol));
                    s->identifier = program->funcdef_node.params[i]->immediateParam.param_name;
                    //printf("here!\n");

                    s->type = ensure_type_exists(program->funcdef_node.params[i]->immediateParam.type);
                    

                    s->scope = assign_symbol_scope(script_mode);
                    s->index = i;
                    s->param_index = i;

                    s->declaration_ctx = get_current_context();
                    s->ctx_block_id = assign_ctx_block_id(script_mode);
                    s->fn_unique_id = -2;
                    add_symbol(f->local_symbols, s);


                    if (s->size > 8)
                    {
                        fprintf(stderr, "In function: %s, parameter: %s, with type: %s, is bigger than 8 bytes. Consider passing a reference to it instead: ptr<%s>\n",
                                f->identifier, s->identifier, s->type, s->type);
                        exit(1);
                    }   

                    // 22/11: Later change to 16

                    continue;
                }

                //Else assume it's a function reference
                Symbol *s = malloc(sizeof(Symbol));
                s->identifier = program->funcdef_node.params[i]->functionParam.identifier;
                s->type = "callback";
                s->scope = assign_symbol_scope(script_mode);
                s->index = i;
                s->param_index = i;


                s->declaration_ctx = get_current_context();
                s->ctx_block_id = assign_ctx_block_id(script_mode);
                s->fn_unique_id = -1;
                s->fn_param_types = program->funcdef_node.params[i]->functionParam.params_type;
                s->fn_param_count = program->funcdef_node.params[i]->functionParam.params_count;
                s->fn_return_type = program->funcdef_node.params[i]->functionParam.return_type;
                add_symbol(f->local_symbols, s);
            }

            // Add the signature
            f->signature = get_function_signature(f);
            // Add the return signature
            f->ret_signature = get_function_ret_signature(f);

            //Analyze the body of the function
            Value v = analyze(program->funcdef_node.body, f->local_symbols, f->local_functions, script_mode);
            //Analyze the return type of the function. Could be void
            compare_ret_type(f->identifier, f->rt_type, v.type);
            
            //We are outside the function, pop the context and scope 
            pop_context();

            //Statements that return nothing
            Value r = {.type = "void"};
            return r;
        }

        case NODE_SKIP:
        {
            //Skip can only be used within a loop 
            if (!current_ctx_is_inside_loop())
            {
                fprintf(stderr, "Keyword 'skip' only makes sense in a loop scope, but raising error usage is in block: %s \n", 
                        ctxToString(get_current_context()));
                exit(1);
            }
            break;
        }

        case NODE_STOP:
        {
            if (!current_ctx_is_inside_loop())
            {
                fprintf(stderr, "Keyword 'stop' only makes sense in a loop scope, but raising error usage is in block: %s \n",
                        ctxToString(get_current_context()));
                exit(1);
            }
            break;
        }

        case NODE_WHILE:
        {
            //Analyze the condition
            analyze(program->while_node.condition_expr, current_st, current_ft, script_mode);

            //Increment current scope, we are inside the while
            push_context(CTX_WHILE); 
            //Analyze the body
            analyze(program->while_node.body, current_st, current_ft,script_mode);
            //analyze(program->while_node.body, current_st, current_ft);

            //Decrement current scope, while loop is finished
            pop_context(); 

            //Statements that return nothing
            Value r = {.type = "void"};
            return r;

        }

        case NODE_IF:
        {
            
            Value condition = analyze(program->if_node.condition_expr, current_st, current_ft, script_mode);
            //Push an IF context 
            push_context(CTX_IF);
            
            Value if_value = analyze(program->if_node.body, current_st, current_ft, script_mode);

            program->if_node.condition_size = size_of_type(condition.type);
            Value else_value = {.type = "void"};
            pop_context();

            if (program->if_node.else_body != NULL)
            {
                push_context(CTX_ELSE); 
                analyze(program->if_node.else_body, current_st, current_ft, script_mode);
                pop_context();

            }

           return if_value;
        }

        case NODE_FOR:
        {

            //Increment current scope, we are inside the for loop
            push_context(CTX_FOR); 
            //Analyze the assignment
            analyze(program->for_node.assignment_expr, current_st, current_ft, script_mode);
            //Analyze the condition
            analyze(program->for_node.condition_expr, current_st, current_ft, script_mode);
            //Analyze the reassignment
            analyze(program->for_node.reassignment_expr, current_st, current_ft, script_mode);
            //Analyze the body
            analyze(program->for_node.body, current_st, current_ft, script_mode);

            //Decrement current scope, we are outside the for loop
            pop_context(); 
            //Statements that return nothing
            Value r = {.type = "void"};
            return r;
        }

        case NODE_FOREACH:
        {
            //Increment current scope, we are inside the foreach loop
            push_context(CTX_FOREACH); 

            // Analyze the declaration, creates iterator
            Value iterator_declaration = analyze(program->foreach_node.foreach_declaration, current_st, current_ft, script_mode);
            program->foreach_node.iterator_type = iterator_declaration.type;

            // Create the counter for the loop
            Symbol *counter = malloc(sizeof(Symbol));
            
            char *buffer2 = malloc(100 * sizeof(char));
            snprintf(buffer2, 100, "__foreach_counter_%i", context.foreach_block_id);

            counter->identifier = buffer2;
            counter->type = "uint32";
            counter->scope = assign_symbol_scope(script_mode);
            counter->index = current_st->count;
            counter->param_index = -1;

            counter->declaration_ctx = get_current_context();
            counter->ctx_block_id = assign_ctx_block_id(script_mode);
            // Save the counter name in the AST 
            program->foreach_node.counter_name = counter->identifier;
            add_symbol(current_st, counter);

            //Analyze the lower and upper limits, must be uint
            Value lower_limit = analyze(program->foreach_node.lower_limit_expr, current_st, current_ft, script_mode);
            Value upper_limit = analyze(program->foreach_node.upper_limit_expr, current_st, current_ft, script_mode);

            if (!is_type_unsigned_int(lower_limit.type))
            {
                fprintf(stderr, "foreach's lower limit must be of type 'uint' but got type '%s' \n", lower_limit.type);
                exit(1);
            }
            if (!is_type_unsigned_int(upper_limit.type))
            {
                fprintf(stderr, "foreach's upper limit must be of type 'uint' but got type '%s' \n", upper_limit.type);
                exit(1);
            }

            // Is the iterable expr actually iterable?
            Value iterable_expr = analyze(program->foreach_node.iterable_expr, current_st, current_ft, script_mode);
            // Store the iterable expression type in the AST
            program->foreach_node.iterable_expr_type = iterable_expr.type;
            //printf(" -------%s\n", program->foreach_node.iterable_expr_type);

            if (!is_type_subscriptable(iterable_expr.type))
            {
                fprintf(stderr, "Src: %s. Foreach node. Iterable expression is not subscriptable, instead is of type: %s\n", tracker.current_src_file,
                        iterable_expr.type);
                exit(1);
            }

            char *element_type;
            if (is_type_array(iterable_expr.type))
            {
                element_type = get_arr_element_type(iterable_expr.type);
            }
            else if (is_type_ptr(iterable_expr.type))
            {
                element_type = get_pointed_type(iterable_expr.type, 1);
            }
            else 
            {
                fprintf(stderr, "FOREACH NODE. I have no idea how we got here, but the iterable expression type is neither ptr or array\n");
                exit(1);
            }


            //Compare the iterator type and pointed type, must be equal
            if (strcmp( element_type , iterator_declaration.type) != 0)
            {
                fprintf(stderr, "Type mismatch in foreach statement. Iterator: '%s' is of type: '%s', but iterable expression contains elements of type: '%s'\n",
                        program->foreach_node.foreach_declaration->declaration_node.identifier, iterator_declaration.type, element_type);
                exit(1);
            }

            analyze(program->foreach_node.body, current_st, current_ft, script_mode);


            //LOWER THE FOREACH NODE FOR BETTER BUILDING AND STORE IT IN THE AST
            program->foreach_node.foreach_lowered = lowerForeach(program);

            //Decrement current scope
            pop_context();
                 
            //Statements that return nothing
            Value r = {.type = "void"};
            return r;

            break;
        }

        case NODE_CAST:
        {
            analyze(program->cast_node.expr, current_st, current_ft, script_mode);
            Value r = {.type = program->cast_node.castType};
            return r;
        }

        case NODE_SIZEOF:
        {
            //TODO: CHANGE INT32 FOR UNSIGNED INT LATER ON
            Value r = {.type = "uint16"};
            if (program->sizeof_node.expr != NULL)
            {
                //Get the type the expression has
                program->sizeof_node.type = analyze(program->sizeof_node.expr, current_st, current_ft, script_mode).type;
            }
            return r;
        }

        case NODE_CB_ASSIGNMENT:
        {
            //Check if the function we are creating a callback of exists
            Function *f = lookup_function(current_ft, context.current_scope, program->cbassignment_node.fn_identifider,
                                       program->cbassignment_node.param_types, program->cbassignment_node.param_count );
            
            Symbol *cb = malloc(sizeof(Symbol));
            cb->fn_unique_id = f->unique_id;
            cb->type = "callback";
            cb->identifier = program->cbassignment_node.cb_identifier;
            cb->fn_return_type = f->rt_type;
            cb->fn_param_types = program->cbassignment_node.param_types;
            cb->fn_param_count = f->param_count;
            cb->index = current_st->count;
            cb->param_index = -1;
            cb->scope = assign_symbol_scope(script_mode);

            cb->declaration_ctx = get_current_context();
            cb->ctx_block_id = assign_ctx_block_id(script_mode);
            add_symbol(current_st,cb);


            Value ret = {.type = "void"};
            return ret;
        }

        case NODE_STDALONE_FUNC_CALL:
        {
            char **param_types = malloc(100 * sizeof(char *));
            for (int i = 0; i < program->funccall_node.params_count; i++)
            {
                param_types[i] = analyze(program->funccall_node.params_expr[i], current_st, current_ft, script_mode).type;
            }
           
                    
            //Try normal function lookup
            Function *f = check_function(current_ft, context.current_scope,
                                          program->funccall_node.identifier, param_types, program->funccall_node.params_count);
            

            // Check if it's a recursive call 
            // Function *current_function = get_CFS();
            // if (current_function == f)
            // {
            //     f->is_recursive = 1;
            // }

            //Populate the funccall params type for correct lookup when generating the bytecode
            program->funccall_node.params_type = param_types;
            if (f == NULL)
            {
                //Check if it's a callback parameter
                Symbol *sym = check_identifier(current_st, context.current_scope, program->funccall_node.identifier);

                if (sym != NULL && strcmp(sym->type, "callback") == 0)
                {
                    if (sym->fn_unique_id == -2)
                    {
                        fprintf(stderr, "Callback %s not bound to a function.\n", sym->identifier);
                        exit(1);
                    }

                    //Verify parameters match
                    for (int i = 0; i < program->funccall_node.params_count; i++)
                    {
                        if (!is_acceptable_param_type(sym->fn_param_types[i], param_types[i]))
                        {
                            fprintf(stderr, "Parameter type mismatch for callback %s \n", sym->identifier);
                            exit(1);
                        }
                    }
                    
                    //printf("In analyzer, STDALONE_FUNC_CALL node, found a callback variable with param_types matching.\n");
                    program->funccall_node.is_callback = 1;
                    //program->funccall_node.callback_func_id = 
                    Value v = {.type = "void"};
                    return v;
                }

                fprintf(stderr, "No function found with name: %s, nor any callback variable that accepts: \n (", program->funccall_node.identifier);
                for (int i = 0; i < program->funccall_node.params_count; i++)
                {
                    printf("%s , ",program->funccall_node.params_type[i]);
                }
                printf(")\n");
                exit(1);
            }

            //Normal function calling


            //Analyze the parameters and compare their types to expected
            Value param;
            Symbol *s;
            for (int i = 0; i < program->funccall_node.params_count; i++)
            {
                //Analyze the parameter
                param.type = param_types[i];
                //Get the expected parameter
                s = get_ParamAtIndex(f->local_symbols, i);
                //Check whether the parameter is callback
                if (param.is_callback)
                {
                    s->fn_unique_id = param.fn_unique_id;
                }

            }

            Value v = {.type = "void"};
            //printf("HEHEHRHERHEE \n");
            return v;
    
        }
        case NODE_FUNC_CALL:
        {
            
            char **param_types = malloc(100 * sizeof(char *));
            for (int i = 0; i < program->funccall_node.params_count; i++)
            {
                param_types[i] = analyze(program->funccall_node.params_expr[i], current_st, current_ft, script_mode).type;
            }



            //Try normal function lookup
            Function *f = check_function(current_ft, context.current_scope,
                                          program->funccall_node.identifier, param_types, program->funccall_node.params_count);
            


            //Populate the funccall params type for correct lookup when generating the code
            program->funccall_node.params_type = param_types;


            if (f == NULL)
            {
                //Check if it's a callback parameter
                Symbol *sym = check_identifier(current_st, context.current_scope, program->funccall_node.identifier);

                if (sym != NULL && strcmp(sym->type, "callback") == 0)
                {
                    if (sym->fn_unique_id == -2)
                    {
                        fprintf(stderr, "Callback %s not bound to a function.\n", sym->identifier);
                        exit(1);
                    }

                    //Verify parameters match
                    for (int i = 0; i < program->funccall_node.params_count; i++)
                    {
                        if (!is_acceptable_param_type(sym->fn_param_types[i], param_types[i]))
                        {
                            fprintf(stderr, "Parameter type mismatch for callback %s \n", sym->identifier);
                            exit(1);
                        }
                    }
                    
                    //printf("In analyzer, FUNC_CALL node, found a callback variable with param_types matching.\n");
                    program->funccall_node.is_callback = 1;
                    //program->funccall_node.callback_func_id = 
                    Value v = {.type = sym->fn_return_type};
                    return v;
                }

                fprintf(stderr, "No function found with name: %s, nor any callback variable that accepts: \n (", program->funccall_node.identifier);
                for (int i = 0; i < program->funccall_node.params_count; i++)
                {
                    printf("%s , ",program->funccall_node.params_type[i]);
                }
                printf(")\n");
                exit(1);
            }

            //Normal function calling
            //Some parameters may be callbacks, gotta check for those to assign the correct 
            //parameter's fn_unique_id to the callback's one 

            //Analyze the parameters and compare their types to expected
            Value param;
            Symbol *s;
            for (int i = 0; i < program->funccall_node.params_count; i++)
            {
                //Analyze the parameter
                param.type = param_types[i];
                //Get the expected parameter
                s = get_ParamAtIndex(f->local_symbols, i);
                //Check whether the parameter is callback
                if (param.is_callback)
                {
                    s->fn_unique_id = param.fn_unique_id;
                }

            }

            Value v = {.type = f->rt_type};
            //printf("HEHEHRHERHEE \n");
            return v;
        }

        case NODE_OBJECT:
        {
            push_context(CTX_OBJECT);
            //Create an object
            Object *o = malloc(sizeof(Object));
            o->identifier = program->object_node.identifier;
            add_object(gb_objectTable, o);


            o->local_symbols = init_object_lc_symbol_table(current_st, program->object_node.declaration_count + 1);  //+ 1 just in case
            //By default, parent is NULL unless it inherits
            o->parent = NULL;
            if (program->object_node.parent != NULL)
            {
                Object *parent = lookup_object(gb_objectTable, program->object_node.parent);
                o->local_symbols->parentST = parent->local_symbols;
                o->parent = parent;
            }

            //Analyze every declaration in the object
            for (int i = 0; i < program->object_node.declaration_count; i++)
            {
                analyze(program->object_node.declarations[i], o->local_symbols, current_ft, script_mode);
            }

            pop_context();
            break;
        }

        // Enums are global to everyone
        case NODE_ENUM:
        { 
            push_context(CTX_ENUM);
            Enum * e = malloc(sizeof(Enum));
            e->identifier = program->enum_node.identifier;
            add_enum(gb_enumTable, e);

            //What to analyze? I guess nothing. It's just identifiers here, and manually assigned to uint8 values by the compiler
            for (int i = 0; i < program->enum_node.declaration_count; i++)
            {
                analyze(program->enum_node.declarations[i], current_st, current_ft, script_mode);
            }
            pop_context();
            break;
        }


        case NODE_BINARY_OP:
        {
            Value l = analyze(program->binary_op_node.left, current_st, current_ft, script_mode);
            Value r = analyze(program->binary_op_node.right, current_st, current_ft, script_mode);
           

            //Addition and subtraction, mult and div, comparison            
            validate_binary_op(program->binary_op_node.op, l.type, r.type);

            return l;
        }

        case NODE_SYSCALL:
        {
            //Don't know what to analyze here
            analyze(program->syscall_node.operand, current_st, current_ft, script_mode);
            //Value to ret for statements with no return value
            Value ret = {.type = "void"};
            return ret;
        }

        case NODE_RETURN:
        {
            Value r;
            if (program->return_node.return_expr == NULL)
            {
                r.type = "void";
                return r;
            }
            r = analyze(program->return_node.return_expr, current_st, current_ft, script_mode);

            return r;

        }

        case NODE_ARRAY_INIT:
        {
            char *last_type = NULL;
            for (int i = 0; i < program->array_init_node.size; i++)
            {
                Value v = analyze(program->array_init_node.elements[i], current_st, current_ft, script_mode);   
                if (last_type != NULL && strcmp(v.type, last_type) != 0)
                {
                    fprintf(stderr, "Type mixing in array initializer: Got '%s' and '%s' \n", last_type , v.type);
                    exit(1);
                }

                last_type = v.type;
            }
            
            char *return_type = malloc(sizeof(char) * 100); 
            sprintf(return_type, "%s [%i]", last_type, program->array_init_node.size);
            Value result = {.type = return_type};
            return result;
        }
        
        case NODE_SUBSCRIPT:
        {

            Value base = analyze(program->subscript_node.base, current_st, current_ft, script_mode);

            if (!is_type_subscriptable(base.type))
            {
                fprintf(stderr, "Src:%s. Array subscription in L=%i. Type: '%s' is not subscriptable.\n", tracker.current_src_file, 
                        program->subscript_node.line_number, base.type);
                exit(1);
            }

            program->subscript_node.base_type = base.type;

            Value index = analyze(program->subscript_node.index, current_st, current_ft, script_mode);

            program->subscript_node.element_size = size_of_type(get_arr_element_type(base.type)); 

            program->subscript_node.index_size = size_of_type(index.type);
            
            if (!is_type_int(index.type))
            {
                fprintf(stderr, "Src: %s . Array subscription in L=%i. Index is expected to be of type integer (signed or unsigned), but got: '%s'\n",
                        tracker.current_src_file, program->subscript_node.line_number, index.type);
                exit(1);
            }


            // We want to return the array element type
            Value result = {.type = get_arr_element_type(base.type)};


            return result;
        }
        
        case NODE_ASSIGNMENT:
        {
            //Create a symbol
            Symbol *s = malloc(sizeof(Symbol ));
            s->identifier = program->assignment_node.identifier;
            s->type = ensure_type_exists(program->assignment_node.type);
            s->index = current_st->count;
            s->param_index = -1;

            s->scope = assign_symbol_scope(script_mode);
            s->declaration_ctx = get_current_context();
            s->ctx_block_id = assign_ctx_block_id(script_mode);
            //Add symbol to table
            add_symbol(current_st, s);
            //Analyze the expression
            Value v = analyze(program->assignment_node.expression, current_st, current_ft, script_mode);
            //Type mismatch between symbol declaration and actual value
            compare_types(s, s->type, v.type);

            //Value to ret for statements with no return value
            Value ret = {.type = "void"};
            return ret;
        }


        case NODE_REASSIGNMENT:
        {
            //Get the symbol to reassign
            Value lvalue = analyze(program->reassignment_node.lvalue, current_st, current_ft, script_mode);

            Value rvalue = analyze(program->reassignment_node.expression,current_st, current_ft, script_mode);


            program->reassignment_node.type = lvalue.type;

            // 23/11 - Dont get the base type (i think). Fucks with 
            // ptr<Node [9 bytes]> p [reassign] -------------- even though the reassignment is 8 bytes, bc of pointer, getting the base type returns Node, which is 9
            //program->reassignment_node.size = size_of_type(get_base_type(lvalue.type));

            program->reassignment_node.size = size_of_type(lvalue.type);

            //printf("Reassingment size = %i \n", program->reassignment_node.size);
            //Analyze the expression
            Value v = analyze(program->reassignment_node.expression,current_st, current_ft, script_mode);

            //printf("Comparing between %s and %s \n", s->type, v.type);
            analyze_types_with_reassign_symbol(lvalue.type, rvalue.type, program->reassignment_node.op);

            //Value to ret for statements with no return value
            Value ret = {.type = "void"};
            //printf("returning from reassignment with type: %s \n",ret.type );
            return ret;
        }

        case NODE_FIELD_ACCESS:
        {


            Value base_type = analyze(program->field_access_node.base, current_st, current_ft, script_mode);
            Object *o = lookup_object(gb_objectTable, base_type.type);

            // Save the type (object identifier)
            program->field_access_node.type = o->identifier;

            // Get the field within the correct symbol table
            Symbol *field = lookup_identifier(o->local_symbols, 1, program->field_access_node.field_name);
            //printf("Field accessed: %s, with type: %s\n", field->identifier, field->type);

            Value ret = {.type = field->type};
            return ret;
        }


        case NODE_PTR_FIELD_ACCESS:
        {
            Value base_type = analyze(program->field_access_node.base, current_st, current_ft, script_mode);

            if ( !is_type_ptr(base_type.type))
            {
                fprintf(stderr, "Base must be of pointer size when performing a field access through pointer \n");
                exit(1);
            }

            //printf("###    NOW HERE \n");

            //What do we want here?
            //char *type = get_base_type(base_type.type);

            char *type = base_type.type;

            char *pointed_type = get_pointed_type(type, 1);

            // Retrieve the object
            Object *o = lookup_object(gb_objectTable, pointed_type);

            // Save the ptr type to the node i guess
            program->ptr_field_access_node.type = base_type.type;

            // Get the field 

            Symbol *field = lookup_identifier(o->local_symbols,1,program->ptr_field_access_node.field_name);

            Value ret = {.type = field->type};
            return ret;
        }
        

        case NODE_IDENTIFIER:
        {

            Symbol *s = lookup_identifier(current_st, context.current_scope, program->identifier_node.name);
            // Special case -----> No symbols in global scope, globals must be known at compile time 
            if (get_current_context() == CTX_NONE)
            {
                fprintf(stderr, 
                "Variable '%s', is not allowed within an expression in global context, which requires everything to be known at compile time.\n", 
                        s->identifier);
                exit(1);
            }

            //printf("Looking up for identifier: %s \n", program->identifier_node.name);
            int is_callback = (strcmp(s->type,"callback") == 0)? 1 : 0; 
            Value v = {.type = s->type, .is_callback = is_callback, .fn_unique_id = s->fn_unique_id};
            return v;
        }

        case NODE_NUMBER:
        {
            char *int_type = get_int_type(program->number_node.number_value);
            program->number_node.int_type = int_type;
            Value number = {.type = int_type};
            return number;
        }

        case NODE_CHAR:
        {
            Value ch = {.type = "char"};
            return ch;
        }

        case NODE_BOOL:
        {
            Value b = {.type = "bool"};
            return b;
        }

        case NODE_UNARY_OP:
        {
            //Some unary operations change the type of the result
            Value result;

            if (strcmp(program->unary_op_node.op, "-") == 0)
            {
                Value number = analyze(program->unary_op_node.right, current_st, current_ft, script_mode);
                if (strcmp(number.type, "uint8") == 0) { result.type = "int8"; }
                else if (strcmp(number.type, "uint16") == 0) { result.type = "int16"; }
                else if (strcmp(number.type, "uint32") == 0) { result.type = "int32"; }
                else { result.type = number.type; }
                program->unary_op_node.size_of_operand = size_of_type(result.type);
                return result;
            }
            //Dereferencing
            if (strcmp(program->unary_op_node.op, "*") == 0)
            {
                Value value_to_deref = analyze(program->unary_op_node.right, current_st, current_ft, script_mode);
                //Special case for strings
                if (strcmp(value_to_deref.type, "str") == 0)
                {
                    result.type = "char";
                    program->unary_op_node.size_of_operand = size_of_type("char");
                    return result;
                }
                
                // 23/11 - commented this, use the resolve_final_ptr_type() function, created for a reason ffs

                //Else we have something like: ptr type. Everytime we dereference, we remove one "ptr" from the type
                // const char *space = strchr(value_to_deref.type, ' ');
                // char *deref_type = strdup(space + 1);
                // result.type = deref_type;
                // program->unary_op_node.size_of_operand = size_of_type(deref_type);

                char *type_pointed_to = resolve_final_ptr_type(value_to_deref.type, 1);
                program->unary_op_node.size_of_operand = size_of_type(type_pointed_to);
                result.type = type_pointed_to;

                return result;
            }

            if (strcmp(program->unary_op_node.op, "&") == 0)
            {
                //Special case like before for chars and str?

                //Else we need to add "ptr" before the type of whatever is pointed to on the right operand
                Value operand = analyze(program->unary_op_node.right, current_st, current_ft, script_mode);
                char *ptr_type = malloc(50 * sizeof(char));
                ptr_type[0] = '\0';
                strcat(ptr_type, "ptr ");
                strcat(ptr_type, operand.type);
                result.type = ptr_type;
                return result;
            }
            result = analyze(program->unary_op_node.right,current_st,current_ft, script_mode);
            program->unary_op_node.size_of_operand = size_of_type(result.type);
            return result;
        }

        case NODE_STDALONE_POSTFIX_OP:
        {
            printf("program->stdalone_postfix_op_node.op = %s \n", program->stdalone_postfix_op_node.op);
            Value operand = analyze(program->stdalone_postfix_op_node.left, current_st, current_ft, script_mode);


            program->stdalone_postfix_op_node.type = operand.type;
            program->stdalone_postfix_op_node.size_of_operand = size_of_type(operand.type);

            Value result = {.type = "void"};
            return result;

        }
        case NODE_POSTFIX_OP:
        {

            Value operand = analyze(program->postfix_op_node.left, current_st, current_ft, script_mode);

            program->postfix_op_node.type = operand.type;
            program->postfix_op_node.size_of_operand = size_of_type(operand.type);


            return operand;
        }   

        case NODE_STR:
        {
            //Add the string to the table and create the label
            add_str_to_table(program->str_node.str_value);

            // Log the label in the AST node
            program->str_node.label = get_last_str_label();

            //printf("Logged a LABEL in the AST: %s \n", program->str_node.label);
            Value str = {.type = "str"};
            return str;
        }

        case NODE_DECLARATION:
        {
            //Create a symbol. Does the type exist?
            Symbol *s = malloc(sizeof(Symbol ));
            s->identifier = program->declaration_node.identifier;

            s->type = ensure_type_exists(program->declaration_node.type);
            s->index = current_st->count;
            s->param_index = -1;
            //s->scope = (gb_symbolTable == current_st) ? 0 : 1;
            s->scope = assign_symbol_scope(script_mode);
             
            s->declaration_ctx = get_current_context();
            s->ctx_block_id = assign_ctx_block_id(script_mode);
            add_symbol(current_st, s);

            //Value to ret for statements with no return value
            Value ret = {.type = s->type};
            return ret;
        }

        default:
        {
            //fprintf(stderr, "Unknown node type to analyze: %i \n", program->node_type);
            //exit(1);
        }

    }
}




void print_symbolTable(SymbolTable *st)
{
    printf("=================================================================\n");
    printf("%-16s %-20s %-8s %-8s %-8s\n", "Name", "Type", "Offset", "Index", "Scope");
    printf("-----------------------------------------------------------------\n");

    for (int i = 0; i < st->count; i++)
    {
        printf("%-16s %-20s %-8i %-8i %-8i\n",
               st->symbols[i]->identifier,
               st->symbols[i]->type,
               st->symbols[i]->offset,
               st->symbols[i]->index,
               st->symbols[i]->scope);
    }

    printf("=================================================================\n");
}


void print_functionTable(FunctionTable *ft)
{
    printf("Name:\t\tRType:\tScope:\tIndex:\t\n");
    
    if (ft->count == 0) { printf("-----------------Empty---------------\n"); return; }
    for (int i = 0; i < ft->count; i++)
    {
        printf("%s\t\t%s\t%i\t%i \n", ft->functions[i]->identifier, ft->functions[i]->rt_type,
               ft->functions[i]->scope, ft->functions[i]->index);
    }
}


void print_tables(FunctionTable *ft)
{
    for (int j = 0; j < ft->count; j++)
    {
        print_symbolTable(ft->functions[j]->local_symbols);
        print_tables(ft->functions[j]->local_functions);

    }

}


void free_symbolTable(SymbolTable *st)
{
    for (int i = 0; i < st->count; i++)
    {
        free(st->symbols[i]);
    }
    
    free(st);
}

void free_functionTable(FunctionTable *ft)
{
    for (int i = 0; i < ft->count; i++)
    {
        free_functionTable(ft->functions[i]->local_functions);
        free(ft->functions[i]);
    }
    
    free(ft);
}



void free_objectTable(ObjectTable *ot)
{
    for (int i = 0; i < ot->count; i++)
    {
        free_symbolTable(ot->objects[i]->local_symbols);
        free(ot->objects[i]);
    }

    free(ot);
}















