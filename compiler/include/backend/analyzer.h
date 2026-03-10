//
// Created by pablo on 28/02/2025.
//

#ifndef ABC_ANALYZER_H
#define ABC_ANALYZER_H

#include "../frontend/ast.h"
#include "tables.h"
#include <stddef.h>
#include <ctype.h>

//Value represents a 'Unit', which has a 'type'
//During analysis we only care about types matching
typedef struct Value
{
    Type *type;
    int size;
}Value;


typedef enum BinOPCode
{
    ADD,
    MUL,
    DIV,
    SUB,
    MOD,
    EQ,
    NEQ,
    SHIFT_R,
    SHIFT_L,
    LT,
    GT,
    LTEQ,
    GTEQ,
    BIT_AND,
    BIT_OR,
    AND,
    OR,
    XOR,
    UNKNOWN
}BinOPCode;


typedef enum UnaryOPCode
{
    UNARY_MINUS,
    UNARY_DEREF,
    UNARY_NOT,
    UNARY_ADDRESS,
    UNARY_BIT_NOT,
    UNARY_UNKNOWN

}UnaryOPCode;

typedef enum PostfixOPCode
{
    POSTFIX_INC,
    POSTFIX_DEC
}PostfixOPCode;


extern SymbolTable *gb_symbolTable;
extern FunctionTable *gb_functionTable;
extern ObjectTable *gb_objectTable;
extern ClassTable *gb_classTable;
extern EnumTable *gb_enumTable;
extern StringTable *gb_stringTable;

extern Context context;
extern LabelContext label_context;


/**
 * ===============================================================================
 * ANALYSIS CONTEXT    
 * ===============================================================================   
 */

void initAnalysisContext();
void resetContext();
char * ctxToString(ContextType ctx_type);
ContextBlock getCurrentContext();
ContextType getCurrentContextType();
int getCurrentScope();
int getCurrentContextBlockId(ContextType contextType);
Class *getCurrentClass();
void initGlobalTables();
void popContext();
void pushContext(ContextType ctx_type);
int contextPushesScope(ContextType current_ctx);
void incrementContextBlockID(ContextType ctx_type);

/**
 * ===============================================================================
 * LOWERERS     
 * ===============================================================================   
 */

 ASTNode *lowerForeach(ASTNode *ast_foreach);
 ASTNode *lowerMethodDispatch(ASTNode *ast_md);
 ASTNode *lowerPtrMethodDispatch(ASTNode *ast_ptrmd);


 /**
 * ===============================================================================
 * BINARY, UNARY AND POSTFIX OPERATIONS     
 * ===============================================================================   
 */

BinOPCode strToBinOPCode(char *str);
BinOPCode tokenAssignToBinOPCode(TokenType tt);
UnaryOPCode strToUnaryOPCode(char *str);
PostfixOPCode strToPostfixOPCode(char *str);

/**
 * ===============================================================================
 * TYPE OPERATIONS     
 * ===============================================================================   
 */

int isTypeAbleToSubscript(Type *type);
int isUDTOfKind(UserDefinedType *udt, UserDefinedTypeKind udt_kind);
Type *getFinalPointedType(Type *type);
int calculateSizeOfType(Type *type);
int isTypeAssignableToDeclared(Type *declaredType, Type *assigningType);
int typesMatchForFunctionCall(Type *type, Type *providedType);
void validateBinaryOP(Type *left, Type *right, BinOPCode bin_op);
void validateUnaryOP(Type *right, char *unary_op);
Type *performUnaryOnType(Type *right, char *unary_op);
void validateReassignOperationWithTypes(Type *lvalue, Type *rvalue, TokenType reassign_symbol);
int doesTypeExist(Type *type);
int areTypesEqual(Type *typeA, Type *typeB);
int checkConflictingUDTs(char *udt_name, UserDefinedTypeKind udt_kind, void *udt);
void populateUDTInfo(UserDefinedTypeKind udt_kind, char *udt_name);
/**
 * ===============================================================================
 * STRING TABLE OPERATIONS     
 * ===============================================================================   
 */


StringTable *__allocateStringTable();
void __resizeStringTable(StringTable *str_table);
StringTable *makeStringTable();
void __initGlobalStringTable();
void addStrToTable(StringTable *str_table, char *str);
Label *getStringTableLabelAt(StringTable *str_table, int index);
void printStringTable();

/**
 * ===============================================================================
 * LABEL OPERATIONS     
 * ===============================================================================   
 */

Tag *__allocateTag();
Tag *makeTag(const char *fmt, ...);
void __resizeIntermediateTagsInCompoundLabel(Label *l);
Label *__allocateLabel();
Label *makeCompoundLabel();
Label *makeTagLabel(char *tag_name);
void setStartTagToCompoundLabel(Label *l, Tag *start_tag);
void setEndTagToCompoundLabel(Label *l, Tag *end_tag);
void addIntermediateTagToCompoundLabel(Label *l, Tag *intermediate_tag);


/**
 * ===============================================================================
 * SYMBOL OPERATIONS     
 * ===============================================================================   
 */

SymbolTable *__allocateSymbolTable();
void __resizeSymbolTable(SymbolTable *st);
SymbolTable *makeSymbolTable(SymbolTable *parentST, SymbolTableKind kind);
void __initGlobalSymbolTable();
SymbolMetaInfo *__allocateSymbolMetaInfo();
Symbol *__allocateSymbol();
SymbolMetaInfo *makeSymbolMetaInfo(char *identifier, Type *type);
Symbol *makeGlobalSymbol(char *identifier, Type *type);
Symbol *makeLocalVarSymbol(char *identifier, Type *type);
Symbol *makeParameterSymbol(char *identifier, int parameter_index, Type *type);
void insertNewParameter(Symbol *parameterToInsert, Parameter **params, int param_count);
Symbol *lookUpSymbol(SymbolTable *st, int scope, char *identifier);
Symbol *lookUpField(SymbolTable *st, char *identifier);
Symbol *lookUpParameter(SymbolTable *st, int param_index);
void addSymbolToTable(Symbol *s, SymbolTable *st);
void absorbSymbolsIntoTable(SymbolTable *absorber_st, SymbolTable *absorbee_st);
int doSymbolsClash(Symbol *s1, Symbol *s2);

/**
 * ===============================================================================
 * FUNCTION OPERATIONS     
 * ===============================================================================   
 */

FunctionTable *__allocateFunctionTable();
void __resizeFunctionTable(FunctionTable *ft);
FunctionTable *makeFunctionTable(FunctionTable *parentFT);
void __initGlobalFunctionTable();
FunctionMetaInfo *__allocateFunctionMetaInfo();
Function *__allocateFunction();
Function *makeMethodFunction(char *name, int param_count, Parameter **parameters, Type *rt_type, Class *owner_class, FunctionTable *parentFT, SymbolTable *parentST);
Function *makeSingleFunction(char *name, int param_count, Parameter **parameters, Type *rt_type, FunctionTable *parentFT, SymbolTable *parentST, int is_forward);
FunctionMetaInfo *makeFunctionMetaInfo(char *name, int param_count, Parameter **parameters, Type *rt_type, FunctionTable *parentFT, SymbolTable *parentST);
void addFunctionToTable(Function *f, FunctionTable *ft);
int doFunctionsClash(Function *f, Function *g);
Function *selectBestFunctionCandidateOf(Function *f1, Function *f2, Type **param_types, int param_count);
Function *lookUpFunction(FunctionTable *ft, int scope, char *identifier, Type **param_types, int param_count);

/**
 * ===============================================================================
 * OBJECT OPERATIONS     
 * ===============================================================================   
 */



Object *__allocateObject();
ObjectTable *__allocateObjectTable();
void __resizeObjectTable(ObjectTable *ot);
ObjectTable *makeObjectTable();
void __initGlobalObjectTable();
Object *makeObject(char *objectName, Object *parentObject, int is_forward, Type *object_type);
Object *lookUpObject(ObjectTable *ot, char *name);
void addObjectToTable(ObjectTable *ot, Object *o);

/**
 * ===============================================================================
 * CLASS OPERATIONS     
 * ===============================================================================   
 */

Class *__allocateClass();
void __resizeClassTable(ClassTable *ct);
ClassTable *__allocateClassTable();
ClassTable *makeClassTable();
void __initGlobalClassTable();
Class *makeClass(char *name, Class *parentClass, int is_forward, Type *class_type);
Class *lookUpClass(ClassTable *ct, char *name);
void addClassToTable(ClassTable *ct, Class *c);

/**
 * ===============================================================================
 * ENUM OPERATIONS     
 * ===============================================================================   
 */

Enum *__allocateEnum();
EnumTable *__allocateEnumTable();
EnumTable *makeEnumTable();
void __initGlobalEnumTable();
void __resizeEnumTable(EnumTable *et);
Enum *makeEnum(char *name, int is_forward, Type *enum_type, Type *base_type);
Enum *lookUpEnum(EnumTable *et, char *name);
void addEnumToTable(EnumTable *et, Enum *e);

/**
 * ===============================================================================
 * ANALYSIS VALUE OPERATIONS     
 * ===============================================================================   
 */


Value *__makeValue();
Value *makeSpecificValue(Type *type);
Value *makeVoidValue();
Value *analyze(ASTNode *program, SymbolTable *current_st, FunctionTable *current_ft);





void print_symbolTable(SymbolTable *st);
void print_functionTable(FunctionTable *ft);
int align_to(int offset, Type *type);
int getAlignmentSize(Type *type);

#endif //ABC_ANALYZER_H
