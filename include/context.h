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
typedef struct Object Object;
typedef struct Label Label;

#define INIT_LABEL_CTX_CAPACITY 10

typedef enum ContextType
{
    CTX_NONE,
    CTX_FUNCTION,
    CTX_OBJECT,
    CTX_UNION,
    CTX_CLASS,
    CTX_ENUM,
    CTX_IF,
    CTX_ELSEIF,
    CTX_ELSE,
    CTX_WHILE,
    CTX_FOR,
    CTX_FOREACH
}ContextType;

typedef struct ContextBlock 
{
    ContextType ctx_type;
    int block_id;
}ContextBlock;

typedef struct Context 
{
    int current_scope;
    int stack_index;
    int enum_block_id;
    int object_block_id;
    int union_block_id;
    int class_block_id;
    int function_block_id;
    int ext_funct_id;
    int func_call_id;
    int if_block_id;
    int elseif_block_id;
    int else_block_id;
    int while_block_id;
    int for_block_id;
    int foreach_block_id;
    int skip_right_id;    // FOR SHORTCIRCUITING && AND ||
    Class *current_class;
    Object *current_object;
    ContextBlock ctx_block_stack[MAX_NESTING];
}Context;



typedef struct LabelContext
{
    int index;
    int count;
    int capacity;
    Label **labels;
}LabelContext;

int isAllowedContext(ContextType ctx_type);
ContextBlock getCurrentContext();
int getCurrentContextBlockId(ContextType ctx_type);
int getCurrentScope();

#endif //ABC_CONTEXT_H