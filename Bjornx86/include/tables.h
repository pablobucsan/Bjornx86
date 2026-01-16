//
// Created by pablo on 03/03/2025.
//



#ifndef ABC_TABLES_H
#define ABC_TABLES_H

#include "types.h"
#include "register.h"
#include "context.h"
#include "parameter.h"

#define MAX_SIZE_STACK 100


#define SYMBOLTABLE_RESIZE_FACTOR 2
#define SYMBOLTABLE_INIT_CAPACITY 10

#define FUNCTIONTABLE_RESIZE_FACTOR 2
#define FUNCTIONTABLE_INIT_CAPACITY 10

#define CLASSTABLE_RESIZE_FACTOR 2
#define CLASSTABLE_INIT_CAPACITY 10

#define OBJECTTABLE_RESIZE_FACTOR 2
#define OBJECTTABLE_INIT_CAPACITY 10

#define ENUMTABLE_RESIZE_FACTOR 2
#define ENUMTABLE_INIT_CAPACITY 10

#define STRINGTABLE_RESIZE_FACTOR 2
#define STRINGTABLE_INIT_CAPACITY 10

#define INTERMEDIATE_TAG_RESIZE_FACTOR 2
#define INTERMEDIATE_TAG_INIT_CAPACITY 10

#define INIT_FUNC_CANDIDATES_CAPACITY 5
#define FUNC_CANDIDATES_RESIZE_FACTOR 2

typedef struct Symbol Symbol;
typedef struct SymbolTable SymbolTable;
typedef struct FunctionMetaInfo FunctionMetaInfo;
typedef struct Function Function;
typedef struct FunctionTable FunctionTable;
typedef struct Class Class;
typedef struct ClassTable ClassTable;
typedef struct Object Object;
typedef struct ObjectTable ObjectTable;
typedef struct Enum Enum;
typedef struct EnumTable EnumTable;

typedef struct FunctionCandidates FunctionCandidates;



typedef struct FunctionCandidates
{
    int count;
    int capacity;
    Function **func_candidates;
}FunctionCandidates;

typedef enum LabelKind
{
    TAG,
    COMPOUND
}LabelKind;

typedef struct Tag
{
    char *tag_name;
}Tag;

typedef struct CompoundLabel
{
    int intermediate_tags_capacity;
    int intermediate_tags_count;
    Tag *start_tag;
    Tag **intermediate_tags;
    Tag *end_tag;
}CompoundLabel;

typedef struct Label
{
    LabelKind kind;
    union{
        Tag tag;
        CompoundLabel compound_label;
    };
}Label;


typedef enum SymbolTableKind
{
    KIND_FUNCTION_ST,
    KIND_OBJECT_ST,
    KIND_CLASS_ST
}SymbolTableKind;


typedef struct SymbolTable
{
    SymbolTableKind kind;
    Symbol **symbols;
    int capacity;
    int weight;
    int padding;
    int count;
    SymbolTable *parentST;
    int scope;
}SymbolTable;

typedef struct FunctionTable
{
    Function **functions;
    int capacity;
    int count;
    int scope;
    FunctionTable *parentFT;
}FunctionTable;

typedef struct ObjectTable
{
    Object **objects;
    int capacity;
    int count;
}ObjectTable;

typedef struct EnumTable 
{
    Enum **enums;
    int capacity;
    int count;
}EnumTable;

typedef struct ClassTable
{
    Class **classes;
    int capacity;
    int count;
}ClassTable;


typedef enum SymbolKind
{
    SYMBOL_GLOBAL,
    SYMBOL_LOCAL
}SymbolKind;

typedef enum LocalVarKind
{
    LOCAL_PARAMETER,
    LOCAL_VAR,
}LocalVarKind;

typedef struct SymbolMetaInfo
{
    Type *type;
    char *identifier;
    int def_line_number;
    char *def_src_file;
    int size;
    int scope;
    int index;
    int offset;
}SymbolMetaInfo;

typedef struct SymbolGlobal
{

}SymbolGlobal;

typedef struct SymbolParameter
{
    int parameter_index;
}SymbolParameter;

typedef struct SymbolLocalVar
{
    ContextType definition_context;
    int context_block_id;
}SymbolLocalVar;

typedef struct SymbolLocal
{
    LocalVarKind local_kind;
    union {
        SymbolParameter symbolParameter;
        SymbolLocalVar symbolLocalVar;
    };
}SymbolLocal;

typedef struct Symbol
{
    SymbolKind symbol_kind;
    SymbolMetaInfo *smi;
    union{
        SymbolGlobal s_global;
        SymbolLocal s_local;
    };
}Symbol;


typedef struct StringTable
{
    int capacity;
    int count;
    char **saved_strings;
    Label **labels;
}StringTable;


typedef struct FunctionMetaInfo
{
    int scope;
    int index;
    int has_varargs;
    int param_count;
    int def_line_number;
    char *def_src_file;
    char *name;
    Parameter **parameters;
    Type *rt_type;
    Label *reference_labels;
    SymbolTable *local_symbols;
    FunctionTable *local_functions;

}FunctionMetaInfo;

typedef struct SingleFunction
{

}SingleFunction;

typedef struct MethodFunction
{
    Class *class_owner;
}MethodFunction;


typedef struct Function
{
    enum{
        FUNCTION_SINGLE,
        FUNCTION_METHOD,
    }kind;

    union{
        MethodFunction method;
        SingleFunction single;
    };

    FunctionMetaInfo *fmi;
}Function;


typedef struct Object
{
    int def_line_number;
    char *identifier;
    char *def_src_file;
    struct Object *parent;
    SymbolTable *local_symbols;
}Object;

typedef struct Enum 
{
    int def_line_number;
    char *def_src_file;
    char *identifier;
}Enum;

typedef struct Class
{
    int def_line_number;
    char *identifier;
    char *def_src_file;
    struct Class *parent;
    SymbolTable *local_symbols;
    FunctionTable *local_functions;
}Class;





#endif //ABC_TABLES_H
