//
// Created by pablo on 03/03/2025.
//

#ifndef ABC_TABLES_H
#define ABC_TABLES_H

typedef struct FunctionTable FunctionTable;
typedef struct SymbolTable SymbolTable;

#define MAX_NESTING 100
#define MAX_SIZE_STACK 100


typedef struct ImmediateParam
{
    char *type;
    char *param_name;
}ImmediateParam;

typedef struct FunctionParam
{
    char *type;             //callback for function params
    char *return_type;
    char *identifier;
    char **params_type;
    int params_count;
}FunctionParam;

typedef struct Param 
{
    enum 
    {
        PARAM_IMMEDIATE,
        PARAM_FUNCTION
    }supertype;

    union
    {
        ImmediateParam immediateParam;
        FunctionParam functionParam;
    };
}Param;


typedef struct FunctionType
{
    char *rt_type;
    char **params_types;
    int param_count;
}FunctionType;


typedef enum SymbolTableKind
{
    KIND_FUNCTION_ST,
    KIND_OBJECT_ST
}SymbolTableKind;

typedef enum ContextType
{
    CTX_NONE,
    CTX_FUNCTION,
    CTX_FUNC_CALL,
    CTX_OBJECT,
    CTX_ENUM,
    CTX_IF,
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
    int function_block_id;
    int func_call_id;
    int if_block_id;
    int else_block_id;
    int while_block_id;
    int for_block_id;
    int foreach_block_id;
    ContextType ctx_type_stack[MAX_NESTING];
}Context;


        

typedef struct Symbol
{
    char *type;                     //i32, char, str, bool
    char *identifier;               //name
    int scope;                      //0 = global, 1 = local
    int index;                      //index within table
    int param_index;                // keeping track of the param index, -1 if its not a param
    int offset;                     // offset in memory
    int size;                       // size of the symbol
    ContextType declaration_ctx;    // Where has this symbol been defined: inside an IF,FOR,WHILE...? 
    int ctx_block_id;               // In what block has it been defined? Two iterators in two consecutive for loops have the same 
                                    // scope and declaration_ctx but have a different ctx_block_id 

    //Symbol could be reference to function
    int fn_unique_id;               //-1 if symbol is immediate, -2 if placeholder for caller function, otherwise function->unique_id
    char *fn_return_type;
    char **fn_param_types;
    int fn_param_count;
}Symbol;

typedef struct StringTable
{
    int size;
    int count;
    char **saved_strings;
    char **labels;
}StringTable;

typedef struct Function
{
    char *rt_type;
    char *identifier;
    char *signature;
    char *ret_signature;
    Param **params;
    SymbolTable *local_symbols;
    FunctionTable *local_functions;
    int index;
    int scope;
    int param_count;
    int unique_id;
    int is_recursive;
}Function;

typedef struct Object
{
    char *identifier;
    struct Object *parent;
    SymbolTable *local_symbols;
}Object;

typedef struct Enum 
{
    char *identifier;
}Enum;


typedef struct SymbolTable
{
    SymbolTableKind kind;
    Symbol **symbols;
    int size;
    int weight;
    int padding;
    int count;
    SymbolTable *parentST;
    int scope;
}SymbolTable;

typedef struct FunctionTable
{
    Function **functions;
    int size;
    int count;
    int scope;
    FunctionTable *parentFT;
}FunctionTable;

typedef struct ObjectTable
{
    Object **objects;
    int size;
    int count;
}ObjectTable;

typedef struct EnumTable 
{
    Enum **enums;
    int size;
    int count;
}EnumTable;


int isAllowedContext(ContextType ctx_type);
ContextType get_current_context();


#endif //ABC_TABLES_H
