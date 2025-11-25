//
// Created by pablo on 28/02/2025.
//

#ifndef ABC_ANALYZER_H
#define ABC_ANALYZER_H

#include "ast.h"
#include "tables.h"



//Value represents a 'Unit', which has a 'type'
//During analysis we only care about types matching
//However, if the type is 'callback', the unit itself
//conveys more information than the type, like func_id
typedef struct Value
{
    char *type;
    //In case the value is type callback
    int is_callback;
    int fn_unique_id;
}Value;

typedef enum SCRIPT_MODE
{
    SCRIPT_KERNEL,
    SCRIPT_USER
}SCRIPT_MODE;

extern int func_id;



extern SymbolTable *gb_symbolTable;
extern FunctionTable *gb_functionTable;
extern ObjectTable *gb_objectTable;
extern EnumTable *gb_enumTable;


extern int start_func_index;
//void analyze(ASTNode *program);

Value analyze(ASTNode *program, SymbolTable *current_st, FunctionTable *current_ft, SCRIPT_MODE script_mode);


void init_gb_symbol_table(int size);
void init_gb_function_table(int size);
void init_gb_object_table(int size);
void init_gb_enum_table(int size);

void init_total_function_array( int size);

Symbol* lookup_identifier(SymbolTable *st, int scope, char *identifier);
//DEPRECATED
Symbol *lookup_identifierByIndex(SymbolTable *st, int index);
//
Symbol *get_ParamAtIndex(SymbolTable *st, int index);
Function *lookup_function(FunctionTable *ft, int scope, char *identifier, char **param_types, int param_count);
Object *lookup_object(ObjectTable *ot, char *identifier);
Object *check_object(ObjectTable *ot, char *identifier);
Function *lookup_function_by_unique_id(int id);
Function *check_function(FunctionTable *ft, int scope, char *identifier, char **param_types, int param_count);
Symbol *check_identifier(SymbolTable *st, int scope, char *identifier);


int is_type_enum(EnumTable *et, char *type);

char **get_paramTypes_from_Params(Param **params, int param_count);
int size_of_type(char *type);
void compare_types(char *identifier, char *declared, char *actual);
char *get_base_type(char *type);
int is_type_ptr(char *str);
char *get_arr_element_type(char *src_type);
char * ctxToString(ContextType ctx_type);
char *resolve_final_ptr_type(const char *ptr_type, int deref_levels);
char *ensure_type_exists(char *type);
char *get_pointed_type(char *pointed_type, int deref_levels);

void print_symbolTable(SymbolTable *st);
void print_functionTable(FunctionTable *ft);
void print_tables(FunctionTable *ft);

void free_symbolTable(SymbolTable *st);
void free_functionTable(FunctionTable *ft);
void free_objectTable(ObjectTable *ot);


#endif //ABC_ANALYZER_H
