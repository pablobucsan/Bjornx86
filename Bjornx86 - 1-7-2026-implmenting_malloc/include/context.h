#ifndef ABC_CONTEXT_H
#define ABC_CONTEXT_H



/**
 * ===============================================================================
 * CONTEXT DEFINITION      
 * ===============================================================================   
 */

#define MAX_NESTING 100
#include "tables.h"
typedef struct Class Class;


typedef enum ContextType
{
    CTX_NONE,
    CTX_FUNCTION,
    CTX_FUNC_CALL,
    CTX_OBJECT,
    CTX_CLASS,
    CTX_ENUM,
    CTX_IF,
    CTX_ELSEIF,
    CTX_ELSE,
    CTX_WHILE,
    CTX_FOR,
    CTX_FOREACH
}ContextType;

typedef struct Context 
{
    int current_scope;
    int stack_index;
    int enum_block_id;
    int object_block_id;
    int class_block_id;
    int function_block_id;
    int func_call_id;
    int if_block_id;
    int elseif_block_id;
    int else_block_id;
    int while_block_id;
    int for_block_id;
    int foreach_block_id;
    Class *current_class;
    ContextType ctx_type_stack[MAX_NESTING];
}Context;


int isAllowedContext(ContextType ctx_type);
ContextType getCurrentContext();
int getCurrentContextBlockId(ContextType ctx_type);
int getCurrentScope();

#endif //ABC_CONTEXT_H