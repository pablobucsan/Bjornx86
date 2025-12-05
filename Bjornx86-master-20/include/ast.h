//
// Created by pablo on 25/02/2025.
//

#ifndef ABC_AST_H
#define ABC_AST_H

#include "tokenizer.h"
#include "tables.h"
#include <stdint.h>

typedef struct ASTNode ASTNode;


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
    ASTNode *program;
    char *filepath;
}UseDirectiveNode;

typedef struct NumberNode
{
    int64_t number_value;
    char *int_type;
}NumberNode;

typedef struct CharNode
{
    char char_value;
}CharNode;

typedef struct StrNode
{
    char *str_value;
    char *label;
}StrNode;

typedef struct BoolNode
{
    char *bool_value;          //true or false
}BoolNode;

typedef struct ObjectInstanceNode
{
    char *instance_identifier;
    char *field_identifier;
}ObjectInstanceNode;

typedef struct UnaryOPNode
{
    char *op;                   // operator ('-', '*', '&;)
    int size_of_operand;        
    ASTNode *right;
} UnaryOPNode;


// Post fix operations bind right-to-left
typedef struct PostfixOPNode
{
    char *op;                   // operator ("++","--")
    int size_of_operand;       
    char *type;
    ASTNode *left;
}PostfixOPNode;



typedef struct StdAlone_PostfixOPNode
{
    char *op;                   // operator ("++","--")
    int size_of_operand;       
    char *type;
    ASTNode *left;
}StdAlone_PostfixOPNode;

typedef struct CastNode
{
    char *castType;
    ASTNode *expr;
} CastNode;

typedef struct SizeOfNode
{
    char *type;
    ASTNode *expr;
}SizeOfNode;

typedef struct BinaryOPNode
{
    char *op;                   // operator                ('+','-','*','/','>',"==")
    ASTNode *left;              // left expression
    ASTNode *right;             // right expression
} BinaryOPNode;

typedef struct IdentifierNode
{
    char *name;
} IdentifierNode;

typedef struct SubscriptNode 
{
    int line_number;
    char *base_identifier;
    ASTNode *base;
    ASTNode *index;
    int element_size;
    int index_size;
    char *base_type;
}SubscriptNode;

typedef struct FieldAccessNode 
{
    ASTNode *base;
    char *type;
    char *field_name;
}FieldAccessNode;

typedef struct PtrFieldAccessNode 
{
    ASTNode *base;
    char *type;
    char *field_name;
}PtrFieldAccessNode;


typedef struct ArrayInitNode 
{
    char *type;
    char *arr_name;
    int size;
    int capacity;
    ASTNode **elements;
}ArrayInitNode;

typedef struct AssignmentNode
{
    int line_number;
    char *type;                 // type of the variable    (i32)
    char *identifier;
    ASTNode *expression;        // expression to evaluate  (2 + 3 * f(1,2) * (-1 + 3) )
} AssignmentNode;

typedef struct ReassignmentNode
{
    int line_number;
    ASTNode *lvalue; // Lvalue
    TokenType op;               // Operation => '=', '+=', '-=', '/=', '*='
    ASTNode *expression;
    int size;
    char *type;
}ReassignmentNode;

typedef struct DeclarationNode
{
    char *type;
    char *identifier;
}DeclarationNode;

typedef struct ObjectNode
{
    char *identifier;
    char *parent;   
    int declaration_count;
    ASTNode **declarations;
}ObjectNode;

typedef struct EnumNode
{
    char *identifier;
    int declaration_count;
    ASTNode **declarations;
}EnumNode;


typedef struct SkipNode
{

}SkipNode;

typedef struct StopNode
{

}StopNode;

typedef struct WhileNode
{
    ASTNode *condition_expr;
    ASTNode *body;
}WhileNode;

typedef struct ForNode
{
    ASTNode *assignment_expr;
    ASTNode *condition_expr;
    ASTNode *reassignment_expr;
    ASTNode *body;
}ForNode;


typedef struct ForeachNode
{
    char *iterator_name;
    char *iterator_type;  

    char *counter_name;

    char *iterable_expr_type;

    ASTNode *foreach_declaration;
    ASTNode *iterable_expr;
    ASTNode *lower_limit_expr;
    ASTNode *upper_limit_expr;
    ASTNode *body;

    ASTNode *foreach_lowered;

}ForeachNode;


typedef struct IfNode
{
    int condition_size;
    ASTNode *condition_expr;
    ASTNode *body;
    ASTNode *else_body;
}IfNode;


typedef struct FuncDefNode
{
    char *return_type;
    char *func_name;
    Param **params;
    int params_count;
    ASTNode *body;
    int fn_unique_id;

    int already_built;                 // for code gen phase
}FuncDefNode;


typedef struct ExternFuncDefNode
{
    char *return_type;
    char *func_name;
    Param **params;
    int params_count;
    int fn_unique_id;
}ExternFuncDefNode;


typedef struct ReturnNode
{
    ASTNode *return_expr;
}ReturnNode;

typedef struct FuncCall
{
    int is_callback;
    char *identifier;
    ASTNode **params_expr;
    int params_count;
    char **params_type;         //to support function overload. To fill up in analyzer
}FuncCall;


typedef struct StdAloneFuncCall
{
    int is_callback;
    char *identifier;
    ASTNode **params_expr;
    int params_count;
    char **params_type;
}StdAloneFuncCall;

typedef struct CBAssignment
{
    char *cb_identifier;
    char *fn_identifider;
    char **param_types;
    int param_count;
}CBAssignment;

typedef struct SysCall
{
    char *syscall_identifier;
    ASTNode *operand;
}SysCall;


typedef struct BlockNode
{
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
        NODE_PTR_REASSIGNMENT,
        NODE_PTR_FIELDREASSIGNMENT,
        NODE_DEREF_FIELD,
        NODE_DECLARATION,
        NODE_UNARY_OP,
        NODE_POSTFIX_OP,
        NODE_STDALONE_POSTFIX_OP,
        NODE_CAST,
        NODE_SIZEOF,
        NODE_BINARY_OP,
        NODE_NUMBER,
        NODE_CHAR,
        NODE_STR,
        NODE_BOOL,
        NODE_SKIP,
        NODE_STOP,
        NODE_INSTANCE,
        NODE_STDALONE_FUNC_CALL,
        NODE_INSTANCE_REASSIGNMENT,
        NODE_FUNC_CALL,
        NODE_SYSCALL,
        NODE_RETURN,
        NODE_NULL,
        NODE_CB_ASSIGNMENT,
        NODE_OBJECT,
        NODE_ENUM,
        NODE_WHILE,
        NODE_IF,
        NODE_FOR,
        NODE_ARRAY_INIT,
        NODE_FOREACH,
        NODE_SUBSCRIPT,
        NODE_FIELD_ACCESS,
        NODE_PTR_FIELD_ACCESS,
        NODE_FUNC_DEF,
        NODE_EXTERN_FUNC_DEF,
        NODE_USE_DIRECTIVE,
        NODE_IDENTIFIER,
        NODE_BLOCK
    } node_type;

    union
    {
        AssignmentNode assignment_node;
        ReassignmentNode reassignment_node;
        DeclarationNode declaration_node;
        ForNode for_node;
        ForeachNode foreach_node;
        WhileNode while_node;
        SkipNode skip_node;
        StopNode stop_node;
        StdAloneFuncCall stdalone_func_call_node;
        IfNode if_node;
        CBAssignment cbassignment_node;
        FuncDefNode funcdef_node;
        ExternFuncDefNode extern_func_def_node;
        FuncCall funccall_node;
        ArrayInitNode array_init_node;
        SysCall syscall_node;
        ReturnNode return_node;
        UseDirectiveNode use_node;
        UnaryOPNode unary_op_node;
        PostfixOPNode postfix_op_node;
        StdAlone_PostfixOPNode stdalone_postfix_op_node;
        CastNode cast_node;
        SizeOfNode sizeof_node;
        BinaryOPNode binary_op_node;
        ObjectNode object_node;
        EnumNode enum_node;
        FieldAccessNode field_access_node;
        PtrFieldAccessNode ptr_field_access_node;
        SubscriptNode subscript_node;
        IdentifierNode identifier_node;
        NumberNode number_node;
        CharNode char_node;
        StrNode str_node;
        BoolNode bool_node;
        ObjectInstanceNode instance_node;
        BlockNode block_node;
        NullNode null_node;
    };
} ASTNode;


ASTNode *cloneAST(ASTNode *original);
ASTNode *makeAST();
ASTNode *makeIdentifier(char *identifier);
ASTNode *makeAssignment(char *type, char *identifier, ASTNode *expression);
ASTNode *makeReassignment(ASTNode *lvalue, TokenType op, ASTNode *expression, int size, char *type);
ASTNode *makeStdalonePostFix(ASTNode *left, char *type, int size_of_operand, char *op);
ASTNode *makeDeclaration(char *type, char *identifier);
ASTNode *makeSubscript(ASTNode *base, ASTNode *index, char *base_type, int element_size, int index_size);
ASTNode *makeWhile(ASTNode *condition_expr, ASTNode *body);
ASTNode *makeBinOP(ASTNode *left, ASTNode *right, char *op);
ASTNode *makeBlock(ASTNode **statements, int count, int capacity);

char *read_file(const char *filename);
void init_usedFiles(int initial_capacity);
void free_usedFiles();

ASTNode *parseProgram(Token **tokens, int *token_pos);

ASTNode *parseStatement(Token **tokens, int *token_pos);

ASTNode *parseUseDirective(Token **tokens,int *token_pos);

Param *parseFunctionParameter(Token **tokens, int *token_pos);

ASTNode *parseLValue(Token **tokens, int *token_pos);



char *resolveType(Token **tokens, int *token_pos);
char *resolvePtrType(Token **tokens, int *token_pos);
ASTNode *parsePtrAssignment(Token **tokens, int *token_pos);
ASTNode *parseAssignment(Token **tokens, int *token_pos);

ASTNode *parseReassignment(Token **tokens, int *token_pos);
ASTNode *parseDeclaration(Token **tokens, int *token_pos);

ASTNode *parseArrayInit(Token **tokens, int *token_pos, char *arr_name);

ASTNode *parseExpression(Token **tokens, int *token_pos);
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

ASTNode *parseLValue_Precedence2(Token **tokens, int *token_pos);


int64_t foldBinOperation(int64_t left, int64_t right, char *binary_op);
int64_t foldUnaryOperation(int64_t right, char *unary_op);
ASTNode *tryFoldBinary(ASTNode *left, ASTNode *right, char *op);
ASTNode *tryFoldUnary(ASTNode *right, char *op);
char *tryResolveType(Token **tokens, int *token_pos);

void print_ast(ASTNode *program, int indent);
void free_ast(ASTNode *node);

int is_type_array(char *type);
char *astTypeToStr(ASTNode *node);
int is_statement(ASTNode *node);
int is_pushing_scope(ASTNode *node);

#endif //ABC_AST_H
