//
// Created by pablo on 25/02/2025.
//

#include "../include/tokenizer.h"
#include <ctype.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define INITIAL_TOKENS_SIZE 100
#define GROWTH_FACTOR 2

char *types[12] = {"int32", "int16","int8", "uint8", "uint16" , "uint32" ,"char", "str", "bool", "void" ,"ptr", "any"};
char *keywords[] = {"while", "for","foreach","in","within","func", "true", "false", "object", "enum" , "return", "if", "else", "use","inherits", "sizeof","callback", "stop", "skip", "extern" };

char *syscalls[] = {"SYSCALL_PRINT", "SYSCALL_BMALLOC", "SYSCALL_CATCH_1", "SYSCALL_CATCH_2" , "SYSCALL_CATCH_4", "SYSCALL_BFREE", "SYSCALL_THROW",
                    "SYSCALL_CREATE_THREAD", "SYSCALL_READ"};


char char_operators[] = {'+', '-', '%', '*', '/', '<', '>', '&', '!'};
char char_symbols[] = {'(', ')', '{', '}', ';', ',', '.', '#'};
char *str_operators[] = {"<=", ">=", "==", "&&", "||", "!="};
char *reassign_symbols[5] = {"-=","+=","*=","/=","%="};


void skip_comment(char **src)
{
    // Skip everything until the end of the line
    while (**src && **src != '\n') {
        (*src)++;
    }
}

int is_operator(char c)
{
    for (int i = 0; i < sizeof(char_operators)/ sizeof(char_operators[0]); i++)
    {
        if (char_operators[i] == c)
        {
            return 1;
        }
    }
    return 0;
}

int is_multi_char_operator(char *src)
{
    char buffer[3] = {src[0], src[1], '\0'};
    for (int i = 0; i < sizeof(str_operators) / sizeof(str_operators[0]); i++)
    {
        if (strcmp(buffer, str_operators[i]) == 0)
        {
            return 1;
        }
    }
    return 0;
}

int is_reassign_symbol(char *src)
{
    char buffer[3] = {src[0],src[1],'\0'};
    for (int i = 0; i < sizeof(reassign_symbols) / sizeof(reassign_symbols[0]); i++)
    {
        if (strcmp(buffer, reassign_symbols[i]) == 0) { return 1; }
    }

    return 0;
}

int is_type(char *buffer)
{
    for (int i = 0; i < sizeof(types)/ sizeof(types[0]); i++)
    {
        if (strcmp(types[i], buffer) == 0)
        {
            return 1;
        }
    }
    return 0;
}

int is_keyword(char *buffer)
{
    for (int i = 0; i < sizeof(keywords)/ sizeof(keywords[0]); i++)
    {
        if (strcmp(keywords[i], buffer) == 0)
        {
            return 1;
        }
    }
    return 0;
}

int is_syscall(char *buffer)
{
    for (int i = 0; i < sizeof(syscalls)/ sizeof(syscalls[0]); i++)
    {
        
        if (strcmp(syscalls[i], buffer) == 0)
        {
              return 1;
        }
    }
    return 0;
}

int is_symbol(char c)
{
    for (int i = 0; i < sizeof(char_symbols)/ sizeof(char_symbols[0]); i++)
    {
        if (char_symbols[i] == c)
        {
            return 1;
        }
    }
    return 0;
}

char *tokenTypeToStr(TokenType type)
{
    switch (type)
    {
        case TOKEN_TYPE: return "TYPE";
        case TOKEN_KEYWORD: return "KEYWORD";
        case TOKEN_NUMBER: return "NUMBER";
        case TOKEN_CHAR: return "CHAR";
        case TOKEN_STR: return "STR";
        case TOKEN_IDENTIFIER: return "IDENTIFIER";
        case TOKEN_SYMBOL: return "SYMBOL";
        case TOKEN_ADD_ASSIGN: return "ADD_ASSIGN";
        case TOKEN_ASSIGN: return "ASSIGN";
        case TOKEN_SUB_ASSIGN: return "SUB_ASSIGN";
        case TOKEN_MOD_ASSIGN: return "MOD_ASSIGN";
        case TOKEN_MUL_ASSIGN: return "MUL_ASSIGN";
        case TOKEN_DIV_ASSIGN: return "DIV_ASSIGN";
        case TOKEN_OPERATOR: return "OPERATOR";
        case TOKEN_SYSCALL: return "SYSCALL";
        case TOKEN_EOF: return "EOF";

        default: return "UNKNOWN TOKEN_TYPE";

    }
}

Token *create_token(TokenType type, const char *value)
{
    Token *new_tk = malloc(sizeof(Token));
    new_tk->tk_type = type;
    new_tk->tk_value = strdup(value);
    return new_tk;
}


Token *read_number(char **src)
{

    char buffer[32];
    int index = 0;
    while (isdigit(**src))
    {
        buffer[index++] = **src;
        (*src)++;
    }

    buffer[index] = '\0';

    return create_token(TOKEN_NUMBER, buffer);
}

Token *read_operator(char **src)
{
    char buffer[2] = {**src, '\0'};
    (*src)++;
    return create_token(TOKEN_OPERATOR, buffer);
}

Token *read_multi_char_operator(char **src)
{
    char buffer[3] = {(*src)[0], (*src)[1], '\0'};
    (*src) += 2; // Move the pointer forward by 2 characters
    return create_token(TOKEN_OPERATOR, buffer);
}

Token *read_reassign_symbol(char **src)
{
    char buffer[3] = {(*src)[0], (*src)[1], '\0'};
    (*src) += 2;
    if (strcmp(buffer, "+=") == 0) { return create_token(TOKEN_ADD_ASSIGN, buffer); }
    if (strcmp(buffer, "-=") == 0) { return create_token(TOKEN_SUB_ASSIGN, buffer); }
    if (strcmp(buffer, "*=") == 0) { return create_token(TOKEN_MUL_ASSIGN, buffer); }
    if (strcmp(buffer, "/=") == 0) { return create_token(TOKEN_DIV_ASSIGN, buffer); }
    if (strcmp(buffer, "%=") == 0) { return create_token(TOKEN_MOD_ASSIGN, buffer); }

    // This should never happen if is_reassign_symbol() is called first
    fprintf(stderr, "Invalid reassign symbol: %s\n", buffer);
    exit(1);
}

Token *read_symbol(char **src)
{
    char sy[2] = {**src, '\0'};
    (*src)++;
    return create_token(TOKEN_SYMBOL, sy);
}

Token *read_char(char **src)
{
    // Advance past the opening quote
    (*src)++;

    // Check for escape sequences
    char ch;
    if (**src == '\\')
    {
        // Handle escape sequences
        (*src)++; // Move past the backslash
        switch (**src)
        {
            case 'n':  ch = '\n'; break;
            case 't':  ch = '\t'; break;
            case '\\': ch = '\\'; break;
            case '\'': ch = '\''; break;
            case '"':  ch = '\"'; break;
            case '0':  ch = '\0'; break;
            default:
                fprintf(stderr, "Unsupported escape sequence: \\%c\n", **src);
                exit(1);
        }
        (*src)++; // Move past the escape sequence character
    }
    else
    {
        // Handle regular characters
        ch = **src;
        (*src)++; // Move past the character
    }

    // Check for the closing quote
    if (**src != '\'')
    {
        fprintf(stderr, "Char literal is longer than 1 character or missing closing quote: %c\n", ch);
        exit(1);
    }
    (*src)++; // Move past the closing quote

    // Create the token
    char ch_str[2] = {ch, '\0'};
    return create_token(TOKEN_CHAR, ch_str);
}

Token *read_str(char **src)
{
    // Advance past the opening double quote
    (*src)++;

    // Allocate a buffer for the string
    char *buffer = malloc(32);
    int index = 0;

    while (**src != '\"')
    {
        if (**src == '\\')
        {
            // Handle escape sequences
            (*src)++; // Move past the backslash
            switch (**src)
            {
                case 'n':  buffer[index++] = '\n'; break;
                case 't':  buffer[index++] = '\t'; break;
                case '\\': buffer[index++] = '\\'; break;
                case '\"': buffer[index++] = '\"'; break;
                case '\'': buffer[index++] = '\''; break;
                case '0':  buffer[index++] = '\0'; break;
                default:
                    fprintf(stderr, "Unsupported escape sequence: \\%c\n", **src);
                    exit(1);
            }
            (*src)++; // Move past the escape sequence character
        }
        else
        {
            // Handle regular characters
            buffer[index++] = **src;
            (*src)++;
        }

        // Resize the buffer if necessary
        if (index >= 31)
        {
            buffer = realloc(buffer, 2 * index);
        }
    }

    // Null-terminate the string
    buffer[index] = '\0';

    // Advance past the closing double quote
    (*src)++;

    // Create the token
    return create_token(TOKEN_STR, buffer);

}

Token *read_identifier(char **src)
{
    char buffer[32];
    int index = 0;
    while (isalpha(**src) || isdigit(**src) || (**src) == '_')
    {
        buffer[index++] = **src;
        (*src)++;
    }
    buffer[index] = '\0';

    if (is_type(buffer)) return create_token(TOKEN_TYPE, buffer);

    if (is_keyword(buffer)) return create_token(TOKEN_KEYWORD, buffer);

    if (is_syscall(buffer)) return create_token(TOKEN_SYSCALL, buffer);

    return create_token(TOKEN_IDENTIFIER, buffer);

}

Token **tokenize(char *src)
{
    size_t capacity = INITIAL_TOKENS_SIZE;
    size_t size = 0;

    // Allocate initial memory for tokens
    Token **tokens = malloc(capacity * sizeof(Token *));
    while (*src)
    {
        if (size >= capacity)
        {
            capacity *= GROWTH_FACTOR;
            tokens = realloc(tokens, capacity * sizeof(Token *) );
            if (!tokens) { fprintf(stderr, "Realloc for tokens failed! \n"); exit(1); }
        }


        if (isspace(*src)){ src++; }
        else if (*src == '/' && *(src + 1) == '/') { skip_comment(&src); }
        else if (isdigit(*src)) { tokens[size++] = read_number(&src); }
        else if (isalpha(*src) || *src == '_') { tokens[size++] = read_identifier(&src); }
        else if (*src == '\'') { tokens[size++] = read_char(&src); }
        else if (*src == '\"') {tokens[size++] = read_str(&src); }
        else if (*src == '=' && *(src + 1) != '=') { tokens[size++] = create_token(TOKEN_ASSIGN, "="); src++; }
        else if (is_reassign_symbol(src)) { tokens[size++] = read_reassign_symbol(&src); }
        else if (is_multi_char_operator(src)) { tokens[size++] = read_multi_char_operator(&src); }
        else if (is_operator(*src)){ tokens[size++] = read_operator(&src); }
        else if (is_symbol(*src)){ tokens[size++] = read_symbol(&src); }
        else
        {
            fprintf(stderr, "Unsupported character to tokenize: %c \n", *src);
            exit(1);
        }

    }
    // Add the EOF token
    if (size >= capacity)
    {
        capacity += 1; // Ensure space for the EOF token
        tokens = realloc(tokens, capacity * sizeof(Token *));
        if (!tokens)
        {
            fprintf(stderr, "Memory reallocation failed\n");
            exit(1);
        }
    }
    tokens[size++] = create_token(TOKEN_EOF, "EOF");

    // Resize the array to fit exactly the number of tokens
    tokens = realloc(tokens, size * sizeof(Token *));
    if (!tokens)
    {
        fprintf(stderr, "Memory reallocation failed\n");
        exit(1);
    }

    return tokens;
}

void print_tokens(Token **tokens)
{
    int index = 0;
    while (tokens[index]->tk_type != TOKEN_EOF)
    {
        printf("(%s : %s)\n",tokenTypeToStr(tokens[index]->tk_type), tokens[index]->tk_value);
       index++;
    }
    printf("(%s : %s)\n",tokenTypeToStr(tokens[index]->tk_type), tokens[index]->tk_value);
    

}



