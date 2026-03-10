//
// Created by pablo on 25/02/2025.
//

#ifndef ABC_AST_H
#define ABC_AST_H

#include "tokenizer.h"
#include "types.h"
#include "../backend/operand.h"
#include "parameter.h"
#include "../backend/tables.h"
#include <stdint.h>


#define INIT_USE_FILES_CAPACITY 5
#define USEDFILES_RESIZE_FACTOR 2

#define INIT_BLOCK_STMT_CAPACITY 10
#define STMTBLOCK_RESIZE_FACTOR 2

#define INIT_CLASS_DECL_CAPACITY 10
#define INIT_CLASS_FUNCDEF_CAPACITY 10
#define CLASS_STMT_RESIZE_FACTOR 2

#define INIT_ENUM_DECL_CAPACTITY 10
#define ENUM_DECL_RESIZE_FACTOR 2

#define INIT_OBJECT_DECL_CAPACITY 10
#define OBJECT_DECL_RESIZE_FACTOR 2

#define INIT_UNION_DECL_CAPACITY 10
#define UNION_DECL_RESIZE_FACTOR 2

#define INIT_FUNCDEF_PARAM_CAPACITY 10
#define FUNCDEF_PARAM_RESIZE_FACTOR 2

#define INIT_FUNCCALL_PARAM_CAPACITY 10
#define FUNCCALL_PARAM_RESIZE_FACTOR 2

#define INIT_ELSEIF_CAPACITY 5
#define IF_ELSEIF_RESIZE_FACTOR 2

#define INIT_ARRINIT_CAPACITY 10
#define ARRINIT_RESIZE_FACTOR 2

typedef struct ASTNode ASTNode;
typedef struct Label Label;
typedef struct Tag Tag;




/**
 * ===============================================================================
 * AST DEFINITION      
 * ===============================================================================   
 */


typedef enum StmtKind
{
    STMT_EMPTY,
    STMT_USE_DIRECTIVE,
    STMT_FUNC_DEF,
    STMT_CONTINUE,
    STMT_BREAK,
    STMT_CLASS_DEF,
    STMT_OBJECT_DEF,
    STMT_UNION,
    STMT_ENUM_DEF,
    STMT_EXT_FUNC_DEF,
    STMT_FW_FUNC_DEF,
    STMT_FW_OBJ_DEF,
    STMT_FW_CLASS_DEF,
    STMT_EXT_ID_DEF,
    STMT_FOR_LOOP,
    STMT_WHILE_LOOP,
    STMT_FOREACH_LOOP,
    STMT_IF,
    STMT_RETURN,
    STMT_FUNC_CALL,
    STMT_DECLARATION,
    STMT_ASSIGNMENT,
    STMT_REASSIGNMENT,
    STMT_EXPRESSION
}StmtKind;

typedef enum UnitKind
{
    UNIT_FUNC_CALL,
    UNIT_CHAR,
    UNIT_BOOL,
    UNIT_NUMBER,
    UNIT_STR,
    UNIT_IDENTIFIER,
    UNIT_SIZEOF,
    UNIT_CAST,
    UNIT_PARENTHESIS,
    UNIT_UNARYOP
}UnitKind;

typedef struct UsedFiles
{
    char **files;
    int count;
    int capacity;
}UsedFiles;

extern UsedFiles *usedFiles;

typedef struct NullNode
{

}NullNode;

typedef struct UseDirectiveNode
{
    int line_number;
    ASTNode *program;
    char *filepath;
}UseDirectiveNode;

typedef struct NumberNode
{
    enum 
    {
        HEX,
        DECIMAL
    }representation;
    int line_number;
    uint64_t number_value;
    Type *number_type;
}NumberNode;

typedef struct CharNode
{
    int line_number;
    char char_value;
    Type *char_type;
}CharNode;

typedef struct StrNode
{
    int line_number;
    char *str_value;
    Label *label;
    Type *str_type;
}StrNode;

typedef struct BoolNode
{
    int line_number;
    char *bool_value;          //true or false
    Type *bool_type;
}BoolNode;


typedef struct UnaryOPNode
{
    int line_number;
    char *op;                   // operator ('-', '*', '&;)
    int size_of_operand;        
    ASTNode *right;
} UnaryOPNode;


// Post fix operations bind right-to-left
typedef struct PostfixOPNode
{
    int line_number;
    char *op;                   // operator ("++","--")
    int size_of_operand;       
    Type *type;
    ASTNode *left;
}PostfixOPNode;

typedef struct CastNode
{
    int line_number;
    Type *castType;
    ASTNode *expr;
} CastNode;

typedef struct SizeOfNode
{
    enum{
        SIZEOF_TYPE,
        SIZEOF_EXPR,
    }sizeof_type;

    int line_number;
    union{
        Type *type;
        ASTNode *expr;
    };

    Type *result_type;

}SizeOfNode;

typedef struct BinaryOPNode
{
    int line_number;
    char *op;                   // operator                ('+','-','*','/','>',"==")
    ASTNode *left;              // left expression
    ASTNode *right;             // right expression
} BinaryOPNode;

typedef struct IdentifierNode
{
    int line_number;
    char *name;
    Symbol *s;
} IdentifierNode;

typedef struct SubscriptNode 
{
    int line_number;
    char *base_identifier;
    ASTNode *base;
    ASTNode *index;
    int element_size;
    int index_size;
    Type *base_type;
}SubscriptNode;

typedef struct FieldAccessNode 
{
    int line_number;
    ASTNode *base;
    Type *type;
    char *field_name;
}FieldAccessNode;

typedef struct PtrFieldAccessNode 
{
    int line_number;
    ASTNode *base;
    Type *type;
    char *field_name;
}PtrFieldAccessNode;

typedef struct MethodDispatch
{
    int line_number;
    ASTNode *base;
    Type *base_type;
    ASTNode *func_call;
    ASTNode *md_lowered;

}MethodDispatch;

typedef struct PtrMethodDispatch
{
    int line_number;
    ASTNode *base;
    Type *base_type;
    ASTNode *func_call;
    ASTNode *ptrmd_lowered;

}PtrMethodDispatch;

typedef struct ArrayInitNode 
{
    Type *type;
    Symbol *arr_symbol;
    ASTNode **elements;
    char *arr_name;
    int line_number;
    int element_count;
    int capacity;
}ArrayInitNode;

typedef struct AssignmentNode
{
    int line_number;
    Type *type;                 // type of the variable    (i32)
    char *identifier;
    ASTNode *expression;        // expression to evaluate  (2 + 3 * f(1,2) * (-1 + 3) )
    Symbol *s;
} AssignmentNode;

typedef struct ReassignmentNode
{
    int line_number;
    ASTNode *lvalue; // Lvalue
    TokenType op;               // Operation => '=', '+=', '-=', '/=', '*='
    ASTNode *expression;
    Type *type;
    int size;
}ReassignmentNode;

typedef struct DeclarationNode
{
    int line_number;
    Type *type;
    char *identifier;
}DeclarationNode;

typedef struct UnionNode
{
    int line_number;
    int declaration_capacity;
    int declaration_count;
    ASTNode **declarations;

}UnionNode;

typedef struct ObjectNode
{
    int line_number;
    int is_forward;                     // to allow forward declarations
    char *identifier;
    char *parent;   
    int declaration_count;
    int declaration_capacity;
    Type *object_type;
    ASTNode **declarations;
}ObjectNode;

typedef struct ClassNode
{
    int line_number;
    int is_forward;
    char *identifier;
    char *parent;
    int declaration_count;
    int declaration_capacity;
    int funcdefs_count;
    int funcdefs_capacity;
    Type *class_type;
    ASTNode **declarations;
    ASTNode **func_defs;
}ClassNode;

typedef struct EnumNode
{
    int line_number;
    int is_forward;
    char *identifier;
    int declaration_count;
    int declaration_capacity;
    Type *enum_type;
    Type *base_type;
    ASTNode **declarations;
}EnumNode;


typedef struct ContinueNode
{
    int line_number;
    Tag *tag;
}ContinueNode;

typedef struct BreakNode
{
    int line_number;
    Tag *tag;
}BreakNode;

typedef struct WhileNode
{
    int line_number;
    ASTNode *condition_expr;
    ASTNode *body;
    Label *label;
}WhileNode;

typedef struct ForNode
{
    int line_number;
    ASTNode *assignment_expr;
    ASTNode *condition_expr;
    ASTNode *reassignment_expr;
    ASTNode *body;
    Label *label;
}ForNode;


typedef struct ForeachNode
{
    int line_number;
    char *iterator_name;
    Type *iterator_type;  

    char *counter_name;

    Type *iterable_expr_type;

    ASTNode *foreach_declaration;
    ASTNode *iterable_expr;
    ASTNode *lower_limit_expr;
    ASTNode *upper_limit_expr;
    ASTNode *body;

    ASTNode *foreach_lowered;

    Label *label;

}ForeachNode;


typedef struct IfNode
{
    int line_number;
    int condition_size;
    int elseif_count;
    int elseif_capacity;
    ASTNode *condition_expr;
    ASTNode *body;
    ASTNode **elseif_nodes;
    ASTNode *else_body;
    Label *reference_label;
}IfNode;

typedef struct ElseifNode
{
    int line_number;
    int condition_size;
    ASTNode *condition_expr;
    ASTNode *body;
}ElseifNode;

typedef struct FuncDefNode
{
    int line_number;
    int is_forward;                 // to allow forward declarations
    Type *return_type;
    char *func_name;
    Parameter **params;
    int params_count;
    int param_capacity;
    ASTNode *body;
    int already_built;                 // for code gen phase
    Function *function;
}FuncDefNode;


typedef struct ExternIdentifierNode
{
    int line_number;
    char *name;
    Type *type;
}ExternIdentifierNode;

typedef struct ExternFuncDefNode
{
    int line_number;
    Type *return_type;
    char *func_name;
    Parameter **params;
    int params_count;
    int param_capacity;
    int fn_unique_id;
    Function *function;
}ExternFuncDefNode;


typedef struct ReturnNode
{
    int line_number;
    ASTNode *return_expr;
}ReturnNode;

typedef struct FuncCall
{
    int line_number;
    char *identifier;
    ASTNode **params_expr;
    int params_count;
    int param_capacity;
    Type **params_type;         //to support function overload. To fill up in analyzer
    Function *function;
}FuncCall;


typedef struct BlockNode
{
    int line_number;
    ASTNode **statements;
    int statement_count;
    int capacity;
} BlockNode;


typedef struct ASTNode
{
    enum
    {
        NODE_ASSIGNMENT,
        NODE_REASSIGNMENT,
        NODE_DECLARATION,
        NODE_UNARY_OP,
        NODE_POSTFIX_OP,
        NODE_CAST,
        NODE_SIZEOF,
        NODE_BINARY_OP,
        NODE_NUMBER,
        NODE_CHAR,
        NODE_STR,
        NODE_BOOL,
        NODE_CONTINUE,
        NODE_BREAK,
        NODE_FUNC_CALL,
        NODE_RETURN,
        NODE_NULL,
        NODE_OBJECT,
        NODE_CLASS,
        NODE_ENUM,
        NODE_WHILE,
        NODE_IF,
        NODE_ELSEIF,
        NODE_FOR,
        NODE_UNION,
        NODE_ARRAY_INIT,
        NODE_FOREACH,
        NODE_SUBSCRIPT,
        NODE_FIELD_ACCESS,
        NODE_PTR_FIELD_ACCESS,
        NODE_METHOD_DISPATCH,
        NODE_PTR_METHOD_DISPATCH,
        NODE_FUNC_DEF,
        NODE_EXTERN_FUNC_DEF,
        NODE_EXTERN_IDENTIFIER,
        NODE_USE_DIRECTIVE,
        NODE_IDENTIFIER,
        NODE_BLOCK
    } node_type;

    union
    {
        AssignmentNode assignment_node;
        ReassignmentNode reassignment_node;
        DeclarationNode declaration_node;
        UnionNode union_node;
        ForNode for_node;
        ForeachNode foreach_node;
        WhileNode while_node;
        ContinueNode continue_node;
        BreakNode break_node;
        IfNode if_node;
        ElseifNode elseif_node;
        FuncDefNode funcdef_node;
        ExternFuncDefNode extern_func_def_node;
        ExternIdentifierNode extern_identifier_node;
        FuncCall funccall_node;
        ArrayInitNode array_init_node;
        ReturnNode return_node;
        UseDirectiveNode use_node;
        UnaryOPNode unary_op_node;
        PostfixOPNode postfix_op_node;
        CastNode cast_node;
        SizeOfNode sizeof_node;
        BinaryOPNode binary_op_node;
        ObjectNode object_node;
        ClassNode class_node;
        EnumNode enum_node;
        MethodDispatch method_dispatch;
        PtrMethodDispatch ptr_method_dispatch;
        FieldAccessNode field_access_node;
        PtrFieldAccessNode ptr_field_access_node;
        SubscriptNode subscript_node;
        IdentifierNode identifier_node;
        NumberNode number_node;
        CharNode char_node;
        StrNode str_node;
        BoolNode bool_node;
        BlockNode block_node;
        NullNode null_node;
    };

    // 13/12 ---- To know whether an expression or something is a stmt.
    char is_stmt;
    // 15/12 --- ASTNodes that evaluate to a value -> put it here
    // useful for codegen
    Operand *operand;
} ASTNode;





void initASTContext();

/**
 * ===============================================================================
 * FILE OPERATIONS    
 * ===============================================================================   
 */

char *read_file(char *filename);
void __initUsedFiles();
int fileIsAlreadyUsed(char *filename);
void addUsedFile(char *filename);
char *get_system_lib_dir();
char *path_join(const char *dir, const char *file);
/**
 * ===============================================================================
 * TYPES OPERATIONS     
 * ===============================================================================   
 */

int isStrUnsignedInt(char *str);
int isStrUnsignedInt(char *str);
int isTypeUnsignedInt(Type *type);
int getNumberBytes(char *number_type);
int isStringVoidType(char *type_string);
int isStringArrayType(char *type_string);
int isStringPtrType(char *type_string);
int isStringStringType(char *type_string);
int isStringNumberType(char *type_string);
int areTypesEqual(Type *typeA, Type *typeB);
int isTypeInTable(Type *type);
int isTypeOfKind(Type *type, TypeKind kind);
Type *getFinalPointedType(Type *ptr_type);
int isValidSizeofType(Type *type);
int isValidCastType(Type *type);
void addBuiltInTypes();
Type *getBaseType(Type *t);
Type *__addTypeToTable(Type *type);
void __initTypeTable();
void __resizeTypeTable();
Type *__addTypeToTable(Type *type);
Type *__allocateType();
Type *makeVoidType();
Type *makeNumberType(int bytes, int is_unsigned, NumberKind kind);
Type *makePointerType(Type *pointed_type);
Type *makeArrayType(Type *el_type, ArraySizeKind size_kind, int n_elements);
Type *makeUDTType(char *type_string);
Type *parseType(Token **tokens, int *token_pos);
Type *tryParseType(Token **tokens, int *token_pos);
Type *findBuiltinInTable(char *repr);
Type *findUDTInTable(char *name);
char *updateTypeRepresentation(Type *t);
char *typeToString(Type *type);
void printTypeTable();


Type *createNumberTypeFromInfo(int bytes, int is_unsigned, NumberKind nkind);
Type *createPointerTypeFromInfo(Type *pointed_type);
Type *createArrayTypeFromInfo(int n_elements, Type *element_type);

/**
 * ===============================================================================
 * STATEMENT CLASSIFICATION     
 * ===============================================================================   
 */

int isEmptyStatement(Token **tokens, int *token_pos);
int isUseStatement(Token **tokens, int *token_pos);
int isFunctionCall(Token **tokens, int *token_pos);
int isWhileStatement(Token **tokens, int *token_pos);
int isContinueStatement(Token **tokens, int *token_pos);
int isBreakStatement(Token **tokens, int *token_pos);
int isIfStatement(Token **tokens, int *token_pos);
int isForStatement(Token **tokens, int *token_pos);
int isForeachStatement(Token **tokens, int *token_pos);
int isExternFunctionDefStatement(Token **tokens, int *token_pos);
int isFuncDefStatement(Token **tokens, int *token_pos);
int isReturnStatement(Token **tokens, int *token_pos);
int isEnumDefStatement(Token **tokens, int *token_pos);
int isObjectDefStatement(Token **tokens, int *token_pos);
int isClassDefStatement(Token **tokens, int *token_pos);
StmtKind classifyStatement(Token **tokens, int *token_pos);
/**
 * ===============================================================================
 * UNIT CLASSIFICATION     
 * ===============================================================================   
 */
int isChar(Token **tokens, int *token_pos);
int isString(Token **tokens, int *token_pos);
int isNumber(Token **tokens, int *token_pos);
int isBoolean(Token **tokens, int *token_pos);
int isBoolean(Token **tokens, int *token_pos);
int isIdentifier(Token **tokens, int *token_pos);
int isSizeOf(Token **tokens, int *token_pos);
int isCast(Token **tokens, int *token_pos);
int isUnaryOp(Token **tokens, int *token_pos);
int isParenthesis(Token **tokens, int *token_pos);
UnitKind classifyUnit(Token **tokens, int *token_pos);
/**
 * ===============================================================================
 * AST MAKERS     
 * ===============================================================================   
 */

char *mystrdup(const char *src);
ASTNode *cloneAST(ASTNode *original);
ASTNode *__allocateAST();
ASTNode *makeIdentifier(char *identifier);
ASTNode *makeAssignment(Type *type, char *identifier, ASTNode *expression);
ASTNode *makeReassignment(ASTNode *lvalue, TokenType op, ASTNode *expression, Type *type);
ASTNode *makePostFix(ASTNode *left, Type *type, int size_of_operand, char *op);
ASTNode *makeDeclaration(Type *type, char *identifier);
ASTNode *makeSubscript(ASTNode *base, ASTNode *index, Type *base_type, int element_size, int index_size);
ASTNode *makeWhile(ASTNode *condition_expr, ASTNode *body);
ASTNode *makeFor(ASTNode *assignment_expr, ASTNode *condition_expr, ASTNode *reassignment_expr, ASTNode *body);
ASTNode *makeBinOP(ASTNode *left, ASTNode *right, char *op);
ASTNode *makeUnaryOP(ASTNode *right, char *op, int size_of_operand);
ASTNode *makeFuncCall(char *identifier, int params_count, ASTNode **params_expr, Type **params_type);
ASTNode *makeBlock(ASTNode **statements, int count, int capacity);



/**
 * ===============================================================================
 * AST PARSING     
 * ===============================================================================   
 */
ASTNode *parseAssignment(Token **tokens, int *token_pos);
ASTNode *parseProgram(Token **tokens, int *token_pos);
ASTNode *parseBlock(Token **tokens, int *token_pos);
ASTNode *parseClassDef(Token **tokens, int *token_pos);
ASTNode *parseDeclaration(Token **tokens, int *token_pos);
ASTNode *parseEmpty(Token **tokens, int *token_pos);
ASTNode *parseEnumDef(Token **tokens, int *token_pos);
ASTNode *parseExternFuncDef(Token **tokens, int *token_pos);
ASTNode *parseForLoop(Token **tokens, int *token_pos);
ASTNode *parseForeachLoop(Token **tokens, int *token_pos);
ASTNode *parseFuncCall(Token **tokens, int *token_pos);
ASTNode *parseFuncDef(Token **tokens, int *token_pos);
ASTNode *parseIf(Token **tokens, int *token_pos);
ASTNode *parseObjectDef(Token **tokens, int *token_pos);
ASTNode *parseUnion(Token **tokens, int *token_pos);
ASTNode *parseReassignment(Token **tokens, int *token_pos);
ASTNode *parseReturn(Token **tokens, int *token_pos);
ASTNode *parseContinue(Token **tokens, int *token_pos);
ASTNode *parseBreak(Token **tokens, int *token_pos);
ASTNode *parseUseDirective(Token **tokens,int *token_pos);
ASTNode *parseWhileLoop(Token **tokens, int *token_pos);

ASTNode *parseStatement(Token **tokens, int *token_pos);


Parameter *parseFunctionParameter(Token **tokens, int *token_pos);
ASTNode *parseLValue(Token **tokens, int *token_pos);
ASTNode *parseLValue_Precedence2(Token **tokens, int *token_pos);
ASTNode *parseExpression(Token **tokens, int *token_pos);
ASTNode *parseExpr_Precedence11(Token **tokens, int *token_pos);
ASTNode *parseExpr_Precedence10(Token **tokens, int *token_pos);
ASTNode *parseExpr_Precedence9(Token **tokens, int *token_pos);
ASTNode *parseExpr_Precedence8(Token **tokens, int *token_pos);
ASTNode *parseExpr_Precedence7(Token **tokens, int *token_pos);
ASTNode *parseExpr_Precedence6(Token **tokens, int *token_pos);
ASTNode *parseExpr_Precedence5(Token **tokens, int *token_pos);
ASTNode *parseExpr_Precedence4(Token **tokens, int *token_pos);
ASTNode *parseExpr_Precedence3(Token **tokens, int *token_pos);
ASTNode *parseExpr_Precedence2(Token **tokens, int *token_pos);
ASTNode *parseExpr_Precedence1(Token **tokens, int *token_pos);

ASTNode *parseUnit(Token **tokens, int *token_pos);
ASTNode *parseArrayInit(Token **tokens, int *token_pos, char *arr_name, Type *type);
ASTNode *parseChar(Token **tokens, int *token_pos);
ASTNode *parseString(Token **tokens, int *token_pos);
ASTNode *parseNumber(Token **tokens, int *token_pos);
ASTNode *parseBoolean(Token **tokens, int *token_pos);
ASTNode *parseIdentifier(Token **tokens, int *token_pos);
ASTNode *parseSizeOf(Token **tokens, int *token_pos);
ASTNode *parseCast(Token **tokens, int *token_pos);
ASTNode *parseParenthesis(Token **tokens, int *token_pos);
ASTNode *parseUnaryOp(Token **tokens, int *token_pos);


/**
 * ===============================================================================
 * OPTIMIZATIONS  
 * ===============================================================================   
 */

uint64_t foldBinOperation(uint64_t left, uint64_t right, char *binary_op);
uint64_t foldUnaryOperation(uint64_t right, char *unary_op);
ASTNode *tryFoldBinary(ASTNode *left, ASTNode *right, char *op);
ASTNode *tryFoldUnary(ASTNode *right, char *op);



void print_ast(ASTNode *program, int indent);
void free_ast(ASTNode *node);

char *statementToStr(StmtKind stmt_kind);
char *astTypeToStr(ASTNode *node);
int is_statement(ASTNode *node);
int is_unary_op(char *op);
int is_cast(Token **tokens, int *token_pos);
int is_pushing_scope(ASTNode *node);
int isLvalue(ASTNode *ast_node);

#endif //ABC_AST_H
