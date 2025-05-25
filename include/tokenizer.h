//
// Created by pablo on 25/02/2025.
//

#ifndef ABC_TOKENIZER_H
#define ABC_TOKENIZER_H


extern char *types[12];
extern char *reassign_symbols[5];
typedef enum TokenType
{
    TOKEN_TYPE,
    TOKEN_KEYWORD,
    TOKEN_SYSCALL,
    TOKEN_CHAR,
    TOKEN_STR,
    TOKEN_NUMBER,
    TOKEN_SYMBOL,
    TOKEN_ADD_ASSIGN,
    TOKEN_SUB_ASSIGN,
    TOKEN_MUL_ASSIGN,
    TOKEN_DIV_ASSIGN,
    TOKEN_MOD_ASSIGN,
    TOKEN_ASSIGN,
    TOKEN_OPERATOR,
    TOKEN_IDENTIFIER,
    TOKEN_EOF
} TokenType;

typedef struct Token
{
    TokenType tk_type;
    char *tk_value;
} Token;


Token **tokenize(char *src);
void print_tokens(Token **tokens);
char *tokenTypeToStr(TokenType type);


#endif //ABC_TOKENIZER_H
