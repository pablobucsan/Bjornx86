//
// Created by pablo on 25/02/2025.
//

#ifndef ABC_TOKENIZER_H
#define ABC_TOKENIZER_H

#define MAX_IDENTIFIER_LENGTH 100

extern char *types[13];
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

typedef struct Tracker 
{
    int current_line;
    char *current_src_file;
}Tracker;

extern Tracker tracker;


typedef struct Token
{
    TokenType tk_type;
    char *tk_value;
    int line_number;
} Token;

void initTokenizerContext();
Token **tokenize(char *src, char *filePath);
void print_tokens(Token **tokens);
char *tokenTypeToStr(TokenType type);
int is_reassign_symbol(char *src);


#endif //ABC_TOKENIZER_H
