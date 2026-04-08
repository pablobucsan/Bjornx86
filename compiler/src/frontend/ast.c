//
// Created by pablo on 25/02/2025.
//
#define _GNU_SOURCE

#include "../../include/frontend/ast.h"
#include "../../include/misc/errors.h"
#include <stdint.h>
#include <stdlib.h>
#include <ctype.h>
#include <unistd.h>
#include <limits.h>
#include <string.h>
#include <stdio.h>
#include <inttypes.h>


UsedFiles *usedFiles = NULL;
TypeTable *gb_typeTable = NULL;


// int ast_count = 0;

void initASTContext()
{
    // Used files once per
    __initUsedFiles();
    __initTypeTable();
}
/**
 * ===============================================================================
 * FILE OPERATIONS     
 * ===============================================================================   
 */
char *read_file(char *filename)
{
    FILE *file = fopen(filename, "r");
    if (!file) {
        Error *e = invoke(FILE_NOT_FOUND);
        appendMessageToError(e, fetchErrorMsgBasedOnErrorID(e->error_id), filename);
        addError(e);
        return NULL;  // Added return to prevent crash
    }
    
    // Get file size
    fseek(file, 0, SEEK_END);
    long file_size = ftell(file);
    fseek(file, 0, SEEK_SET);  // Rewind to beginning
    
    // Allocate buffer for entire file + null terminator
    char *buffer = malloc(file_size + 1);
    if (!buffer) {
        perror("Memory allocation failed");
        fclose(file);
        return NULL;
    }
    
    // Read entire file in one go
    size_t bytes_read = fread(buffer, 1, file_size, file);
    buffer[bytes_read] = '\0';  // Null-terminate
    
    fclose(file);
    return buffer;
}

void __initUsedFiles(){
    usedFiles = malloc(sizeof(UsedFiles));    
    usedFiles->files = malloc(INIT_USE_FILES_CAPACITY * sizeof(char *));
    usedFiles->count = 0;
    usedFiles->capacity = INIT_USE_FILES_CAPACITY;
}

void __resizeUsedFiles()
{
    usedFiles->capacity = USEDFILES_RESIZE_FACTOR * usedFiles->capacity;
    usedFiles->files = realloc(usedFiles->files, usedFiles->capacity * sizeof(char*));
}


int fileIsAlreadyUsed(char *filename)
{
    char resolved[PATH_MAX];
    if (realpath(filename, resolved) == NULL) {
        // If realpath fails (e.g. file doesn't exist yet), fall back to raw string
        // This is safe because we're only checking for duplicates
        resolved[0] = '\0';
        strncpy(resolved, filename, PATH_MAX-1);
    }

    for (int i = 0; i < usedFiles->count; i++) {
        if (strcmp(resolved, usedFiles->files[i]) == 0) {
            return 1;
        }
    }
    return 0;
}   

void addUsedFile(char *filename)
{
    char resolved[PATH_MAX];
    if (realpath(filename, resolved) == NULL) {
        // Fallback if file doesn't exist yet
        strncpy(resolved, filename, PATH_MAX-1);
        resolved[PATH_MAX-1] = '\0';
    }

    if (usedFiles->count + 1 >= usedFiles->capacity) {
        __resizeUsedFiles();
    }

    usedFiles->files[usedFiles->count] = strdup(resolved);  // strdup is cleaner
    usedFiles->count++;
}

// Return directory part of a path (must free result)
char *get_directory_path(const char *full_path)
{
    char *dir = strdup(full_path);
    char *last_slash = strrchr(dir, '/');
    if (last_slash == NULL) {
        free(dir);
        return strdup(".");
    }
    *last_slash = '\0';  // truncate at last /
    return dir;
}

// Safe path join (must free result)
char *path_join(const char *dir, const char *file)
{
    size_t len1 = strlen(dir);
    size_t len2 = strlen(file);
    char *result = malloc(len1 + len2 + 2);  // +1 for /, +1 for null
    strcpy(result, dir);
    if (dir[len1 - 1] != '/') {
        strcat(result, "/");
    }
    strcat(result, file);
    return result;
}

char *get_system_lib_dir()
{
    char *env_path = getenv("BJORN_LIB_PATH");
    if (env_path != NULL) {
        return env_path;
    }

    char *home = getenv("HOME");
    if (home != NULL) {
        static char sys_lib_buf[PATH_MAX];
        snprintf(sys_lib_buf, sizeof(sys_lib_buf), "%s/.local/lib/bjorn", home);
        return sys_lib_buf;
    }

    return "/usr/local/lib/bjorn";

}


// Simple existence check
int file_exists(const char *path)
{
    FILE *f = fopen(path, "r");
    if (f) {
        fclose(f);
        return 1;
    }
    return 0;
}

/**
 * ===============================================================================
 * END FILE OPERATIONS     
 * ===============================================================================   
 */

int is_assignable_op(Token **tokens, int token_pos)
{
    if (tokens[token_pos]->tk_type == TOKEN_ASSIGN) { return 1; }
    if (tokens[token_pos]->tk_type == TOKEN_ADD_ASSIGN) { return 1; }
    if (tokens[token_pos]->tk_type == TOKEN_SUB_ASSIGN) { return 1; }
    if (tokens[token_pos]->tk_type == TOKEN_MUL_ASSIGN) { return 1; }
    if (tokens[token_pos]->tk_type == TOKEN_MOD_ASSIGN) { return 1; }
    if (tokens[token_pos]->tk_type == TOKEN_DIV_ASSIGN) { return 1; }
    return 0;
}

uint64_t strToNumber(const char *str, char **endPtr)
{
    // Hexadecimal
    if (str[0] == '0' && str[1] == 'x'){
        return strtoull(str, endPtr, 16);
    }

    // Decimal 
    return strtoull(str, endPtr, 10);
}




// Returns 1 and advances to the next token if current token matches expected
int expect_token(Token **tokens, int *token_pos, TokenType type, char *value)
{
    if (tokens[*token_pos]->tk_type == type &&
            strcmp(value, tokens[*token_pos]->tk_value) == 0)
    {
        (*token_pos)++;
        return 1;
    }

    return 0;
}

void demand_token(Token **tokens, int *token_pos, TokenType type, char *value)
{
    if (tokens[*token_pos]->tk_type == type &&
        strcmp(value, tokens[*token_pos]->tk_value) == 0){
        (*token_pos)++;
        return;
    }

    /** Report the error */
    Error *e = invoke(EXPECTED_TOKEN_NOT_MET);
    appendMessageToError(e, fetchErrorMsgBasedOnErrorID(e->error_id), value, tokens[*token_pos]->tk_value);
    addError(e);

    /** Based on what was expected, maybe its fine to continue */
    // if (strcmp(value, ";") == 0 || strcmp(value, ",") == 0 ||
    //     strcmp(value, ")") == 0 || strcmp(value, "]") == 0){
    //     (*token_pos)++;
    // }
    /** Dont consume the token, not safe */
    // else{

    // } 
}

void demand_token_type(Token **tokens, int *token_pos, TokenType type)
{
    // printf("In demand token type, we want a: '%s', and we have: (%s,%s)\n", tokenTypeToStr(type),tokenTypeToStr(tokens[*token_pos]->tk_type), tokens[*token_pos]->tk_value);
    if (tokens[*token_pos]->tk_type == type){
        (*token_pos)++;
        return;
    }        

    /** Report the error */
    Error *e = invoke(EXPECTED_TKTYPE_NOT_MET);
    appendMessageToError(e, fetchErrorMsgBasedOnErrorID(e->error_id), tokenTypeToStr(type), tokens[*token_pos]->tk_value);
    addError(e);
}


/**
 * ===============================================================================
 * STATEMENT CLASSIFICATION     
 * ===============================================================================   
 */

int isEmptyStatement(Token **tokens, int *token_pos)
{
    if (tokens[*token_pos]->tk_type == TOKEN_SYMBOL && 
        strcmp(tokens[*token_pos]->tk_value, ";") == 0){
        return 1;
    }
    return 0;
}

int isUseStatement(Token **tokens, int *token_pos)
{
    if (strcmp(tokens[*token_pos]->tk_value, "#") == 0 && 
        strcmp(tokens[*token_pos + 1]->tk_value, "use") == 0){
        return 1;
    }
    return 0;
}

int isFunctionCall(Token **tokens, int *token_pos)
{
    if (tokens[*token_pos]->tk_type == TOKEN_IDENTIFIER &&
        strcmp(tokens[*token_pos + 1]->tk_value, "(") == 0){
        return 1;
    }

    return 0;
}

int isWhileStatement(Token **tokens, int *token_pos)
{
    if (tokens[*token_pos]->tk_type == TOKEN_KEYWORD &&
        strcmp(tokens[*token_pos]->tk_value, "while") == 0){
        return 1;
    }
    return 0;
}

int isContinueStatement(Token **tokens, int *token_pos)
{
    if (tokens[*token_pos]->tk_type == TOKEN_KEYWORD && 
        strcmp(tokens[*token_pos]->tk_value, "continue") == 0){
        return 1;
    }
    return 0;
}

int isBreakStatement(Token **tokens, int *token_pos)
{
    if (tokens[*token_pos]->tk_type == TOKEN_KEYWORD && 
        strcmp(tokens[*token_pos]->tk_value, "break") == 0){
        return 1;
    }
    return 0;
}

int isIfStatement(Token **tokens, int *token_pos)
{
    if (tokens[*token_pos]->tk_type == TOKEN_KEYWORD &&
        strcmp(tokens[*token_pos]->tk_value, "if") == 0){
        return 1;
    }
    return 0;
}

int isForStatement(Token **tokens, int *token_pos)
{
    if (tokens[*token_pos]->tk_type == TOKEN_KEYWORD &&
        strcmp(tokens[*token_pos]->tk_value, "for") == 0){
        return 1; 
    }
    return 0;
}

int isForeachStatement(Token **tokens, int *token_pos)
{
    if (tokens[*token_pos]->tk_type == TOKEN_KEYWORD &&
        strcmp(tokens[*token_pos]->tk_value, "foreach") == 0){
        return 1; 
    }
    return 0;
}

int isExternFunctionDefStatement(Token **tokens, int *token_pos)
{
    if (tokens[*token_pos]->tk_type == TOKEN_KEYWORD && 
        strcmp(tokens[*token_pos]->tk_value, "extern") == 0 && 
        strcmp(tokens[*token_pos + 1]->tk_value, "func") == 0){
        return 1;
    }
    return 0;
}

int isExternIdentifierStatement(Token **tokens, int *token_pos)
{
    if (strcmp(tokens[*token_pos]->tk_value, "extern") != 0 ){
        return 0;
    }

    int token_pos_copy = *token_pos;
    token_pos_copy++; // past 'extern'
    
    Type *potential_type  = tryParseType(tokens, &token_pos_copy);
    if (potential_type == NULL || !isValidCastType(potential_type)){
        return 0;
    }

    return 1;
}

int isFuncDefStatement(Token **tokens, int *token_pos)
{
    expect_token(tokens, token_pos, TOKEN_KEYWORD, "forward");

    if (strcmp(tokens[*token_pos]->tk_value, "func") == 0) {
        return 1;
    }

    return 0;
}

int isReturnStatement(Token **tokens, int *token_pos)
{
    if (tokens[*token_pos]->tk_type == TOKEN_KEYWORD &&
        strcmp(tokens[*token_pos]->tk_value, "return") == 0) {
        return 1;
    }

    return 0;
}

int isEnumDefStatement(Token **tokens, int *token_pos)
{
    expect_token(tokens, token_pos, TOKEN_KEYWORD, "forward");

    if (tokens[*token_pos]->tk_type == TOKEN_KEYWORD && 
        strcmp(tokens[*token_pos]->tk_value, "enum") == 0) {
        return 1;
    }

    return 0;
}

int isObjectDefStatement(Token **tokens, int *token_pos)
{
    
    expect_token(tokens, token_pos, TOKEN_KEYWORD, "forward");

    if ( (strcmp(tokens[*token_pos]->tk_value, "object") == 0) &&
        ( (strcmp(tokens[*token_pos + 2]->tk_value, "{") == 0 || strcmp(tokens[*token_pos + 2]->tk_value, ";") == 0) ||
        strcmp(tokens[*token_pos + 2]->tk_value,"inherits") == 0 )){
        return 1;
    }

    return 0;
}

int isUnionStatement(Token **tokens, int *token_pos)
{
    if (tokens[*token_pos]->tk_type == TOKEN_KEYWORD && strcmp(tokens[*token_pos]->tk_value, "union") == 0 
        && strcmp(tokens[*token_pos + 1]->tk_value, "{") == 0){
        return 1;
    }
    return 0;
}

int isClassDefStatement(Token **tokens, int *token_pos)
{
    expect_token(tokens, token_pos, TOKEN_KEYWORD, "forward");

    if (tokens[*token_pos]->tk_type == TOKEN_KEYWORD && 
        strcmp(tokens[*token_pos]->tk_value, "class") == 0){
        return 1;
    }
    return 0;
}

StmtKind classifyStatement(Token **tokens, int *token_pos)
{

    int saved_token_pos = *token_pos;
    int token_pos_copy = *token_pos;
    
    /** Empty statement */
    if (isEmptyStatement(tokens, &token_pos_copy)){
        return STMT_EMPTY;
    }
    token_pos_copy = saved_token_pos;
    /** #use statement */
    if (isUseStatement(tokens, &token_pos_copy)){
        return STMT_USE_DIRECTIVE;
    }
    token_pos_copy = saved_token_pos;
    /**  Function call statement */
    if (isFunctionCall(tokens, &token_pos_copy)){
        return STMT_FUNC_CALL;
    }
    token_pos_copy = saved_token_pos;
    /** While statement */
    if (isWhileStatement(tokens, &token_pos_copy)){
        return STMT_WHILE_LOOP;
    }
    token_pos_copy = saved_token_pos;
    /** Continue statement */
    if (isContinueStatement(tokens, &token_pos_copy)){
        return STMT_CONTINUE;
    }
    token_pos_copy = saved_token_pos;
    /** Break statement */
    if (isBreakStatement(tokens, &token_pos_copy)){
        return STMT_BREAK;
    }
    token_pos_copy = saved_token_pos;
    /** If statement */
    if (isIfStatement(tokens, &token_pos_copy)){
        return STMT_IF;
    }
    token_pos_copy = saved_token_pos;
    /** For statement */
    if (isForStatement(tokens, &token_pos_copy)){
        return STMT_FOR_LOOP;
    }
    token_pos_copy = saved_token_pos;
    /** Foreach statement */
    if (isForeachStatement(tokens, &token_pos_copy)){
        return STMT_FOREACH_LOOP;
    }
    token_pos_copy = saved_token_pos;
    /** Foreach statement */
    if (isForeachStatement(tokens, &token_pos_copy)){
        return STMT_FOREACH_LOOP;
    }
    token_pos_copy = saved_token_pos;
    /** Extern function definition statement */
    if (isExternFunctionDefStatement(tokens, &token_pos_copy)){
        return STMT_EXT_FUNC_DEF;
    }
    token_pos_copy = saved_token_pos;
    if (isExternIdentifierStatement(tokens, &token_pos_copy)){
        return STMT_EXT_ID_DEF;
    }
    token_pos_copy = saved_token_pos;
    /** Function definition statement */
    if (isFuncDefStatement(tokens, &token_pos_copy)){
        return STMT_FUNC_DEF;
    }
    token_pos_copy = saved_token_pos;
    /** Return statement */
    if (isReturnStatement(tokens, &token_pos_copy)){
        return STMT_RETURN;
    }
    token_pos_copy = saved_token_pos;
    /** Enum definition statement */
    if (isEnumDefStatement(tokens, &token_pos_copy)){
        return STMT_ENUM_DEF;
    }
    token_pos_copy = saved_token_pos;
    /** Object definition statement */
    if (isObjectDefStatement(tokens, &token_pos_copy)){
        return STMT_OBJECT_DEF;
    }
    token_pos_copy = saved_token_pos;
    /** Union statement  */
    if (isUnionStatement(tokens, &token_pos_copy)){
        return STMT_UNION;
    }
    token_pos_copy = saved_token_pos;
    /** Class definition statement */
    if (isClassDefStatement(tokens, &token_pos_copy)){
        return STMT_CLASS_DEF;
    }

    /** Assignment/Declaration/Reassignment/Expression */

    token_pos_copy = saved_token_pos;
    Type *potential_type = tryParseType(tokens, &token_pos_copy);

    // printf("We tried parse the type: %s\n", potential_type->representation);
    /**
     * We have a type and an identifier now
     */
    // printf("This far in classifuying stmt, current token = %s\n", tokens[*token_pos]->tk_value);
    if (potential_type != NULL && tokens[token_pos_copy]->tk_type == TOKEN_IDENTIFIER){
        Token *next = tokens[token_pos_copy + 1];
        if (next != NULL && (next->tk_type == TOKEN_SYMBOL && strcmp(next->tk_value, ";") == 0) ){
            // printf("Is declaration, current token = %s\n", tokens[*token_pos]->tk_value);
            return STMT_DECLARATION;
        }
        if (next && (next->tk_type == TOKEN_ASSIGN && strcmp(next->tk_value, "=") == 0)){
            return STMT_ASSIGNMENT;
        }
    }

    /**
     * If we get here, is not a declaration nor an assignment
     */

    token_pos_copy = saved_token_pos;
    ASTNode *lvalue = parseLValue(tokens, &token_pos_copy);
    if (lvalue != NULL){
        Token *next = tokens[token_pos_copy + 1];
        if (next && is_assignable_op(tokens, token_pos_copy)){
            return STMT_REASSIGNMENT;
        }
    }

    /**
     * If neither, its an expression
     */
    return STMT_EXPRESSION;  // base->foo(); id++; etc
    

}

/**
 * ===============================================================================
 * END STATEMENT CLASSIFICATION     
 * ===============================================================================   
 */

/**
 * ===============================================================================
 * UNIT CLASSIFICATION     
 * ===============================================================================   
 */

int isChar(Token **tokens, int *token_pos){
    return tokens[*token_pos]->tk_type == TOKEN_CHAR;
}

int isString(Token **tokens, int *token_pos){
    return tokens[*token_pos]->tk_type == TOKEN_STR;
}

int isNumber(Token **tokens, int *token_pos){
    return tokens[*token_pos]->tk_type == TOKEN_NUMBER;
}

int isBoolean(Token **tokens, int *token_pos){
    if (tokens[*token_pos]->tk_type == TOKEN_KEYWORD && (
        strcmp(tokens[*token_pos]->tk_value, "true") == 0 ||
        strcmp(tokens[*token_pos]->tk_value, "false") == 0)){
        return 1;
    }
    return 0;
}

int isIdentifier(Token **tokens, int *token_pos){
    return tokens[*token_pos]->tk_type == TOKEN_IDENTIFIER;
}

int isSizeOf(Token **tokens, int *token_pos){
    if (strcmp(tokens[*token_pos]->tk_value, "sizeof") == 0){
        return 1;
    }
    return 0;
}

int isCast(Token **tokens, int *token_pos){
    int token_pos_copy = *token_pos;
    if (strcmp(tokens[token_pos_copy]->tk_value, "(") != 0){
        return 0;
    }
    token_pos_copy++;
    // printf("Checeking if is cast, current token = %s\n", tokens[token_pos_copy]->tk_value);
    Type *potential_type  = tryParseType(tokens, &token_pos_copy);
    // printf("type == NULL? %i, isvalidcasttype? %i, current tok = %s\n", potential_type == NULL, isValidCastType(potential_type),tokens[token_pos_copy]->tk_value);
    if (potential_type != NULL && isValidCastType(potential_type) && strcmp(tokens[token_pos_copy]->tk_value, ")") == 0){
        return 1;
    }
    // printf("Is not\n");
    return 0;
}

int isUnaryOp(Token **tokens, int *token_pos){
    if (strcmp(tokens[*token_pos]->tk_value, "-") == 0) { return 1; }
    if (strcmp(tokens[*token_pos]->tk_value, "!") == 0) { return 1; }  
    if (strcmp(tokens[*token_pos]->tk_value, "*") == 0) { return 1; }  
    if (strcmp(tokens[*token_pos]->tk_value, "&") == 0) { return 1; }  

    return 0;
}

int isParenthesis(Token **tokens, int *token_pos){
    if (tokens[*token_pos]->tk_type == TOKEN_SYMBOL && 
        strcmp(tokens[*token_pos]->tk_value, "(") == 0){
        return 1;
    }
    return 0;
}

UnitKind classifyUnit(Token **tokens, int *token_pos)
{
    int saved_token_pos = *token_pos;
    int token_pos_copy = *token_pos;

    if (isFunctionCall(tokens, &token_pos_copy)){
        return UNIT_FUNC_CALL;
    }
    token_pos_copy = saved_token_pos;
    if (isChar(tokens, &token_pos_copy)){
        return UNIT_CHAR;
    }
    token_pos_copy = saved_token_pos;
    if (isBoolean(tokens, &token_pos_copy)){
        return UNIT_BOOL;
    }
    token_pos_copy = saved_token_pos;
    if (isNumber(tokens, &token_pos_copy)){
        return UNIT_NUMBER;
    }
    token_pos_copy = saved_token_pos;
    if (isIdentifier(tokens, &token_pos_copy)){
        return UNIT_IDENTIFIER;
    }
    token_pos_copy = saved_token_pos;
    if (isCast(tokens, &token_pos_copy)){
        return UNIT_CAST;
    }
    token_pos_copy = saved_token_pos;
    if (isParenthesis(tokens, &token_pos_copy)){
        return UNIT_PARENTHESIS;
    }
    token_pos_copy = saved_token_pos;
    if (isSizeOf(tokens, &token_pos_copy)){
        return UNIT_SIZEOF;
    }
    token_pos_copy = saved_token_pos;
    if (isString(tokens, &token_pos_copy)){
        return UNIT_STR;
    }
    token_pos_copy = saved_token_pos;
    if (isUnaryOp(tokens, &token_pos_copy)){
        return UNIT_UNARYOP;
    }
    token_pos_copy = saved_token_pos;

    fprintf(stderr, "In file: '%s'. In L = %i. Unknown token to unit classify: %s\n", tracker.current_src_file, tracker.current_line, tokens[*token_pos]->tk_value);
    exit(1);
}

/**
 * ===============================================================================
 * END UNIT CLASSIFICATION     
 * ===============================================================================   
 */


/**
 * ===============================================================================
 * TYPES OPERATIONS     
 * ===============================================================================   
 */

Type *getFinalPointedType(Type *ptr_type)
{
    Type *base = ptr_type;
    while(isTypeOfKind(base, TYPE_PTR)){
        base = base->pointerType.pointed_type;
    }

    return base;
}


int isStrUnsignedInt(char *str) 
{
    return (strcmp(str, "uint8") == 0 || strcmp(str, "uint16") == 0 || strcmp(str, "uint32") == 0 || strcmp(str, "uint64") == 0 
            || strcmp(str, "char") == 0 || strcmp(str, "bool") == 0);
}

int isStrSignedInt(char *str)
{
    return (strcmp(str, "int8") == 0 || strcmp(str, "int16") == 0 || strcmp(str, "int32") == 0 || strcmp(str, "int64") == 0);
}

int getNumberBytes(char *number_type)
{
    if (strcmp(number_type, "char") == 0 || strcmp(number_type, "bool") == 0) { return 1; }   
    if (strcmp(number_type, "uint8") == 0 || strcmp(number_type, "int8") == 0) { return 1; }
    if (strcmp(number_type, "uint16") == 0 || strcmp(number_type, "int16") == 0) { return 2; }
    if (strcmp(number_type, "uint32") == 0 || strcmp(number_type, "int32") == 0) { return 4; }
    if (strcmp(number_type, "uint64") == 0 || strcmp(number_type, "int64") == 0) { return 8; }

    fprintf(stderr, "Cannot assign a rank to a type that is not a number. Got: %s\n", number_type);
    exit(1);
}

int isStringVoidType(char *type_string){
    return strcmp(type_string, "void") == 0;
}

int isStringArrayType(char *type_string)
{
    for (int i = 0; type_string[i] != '\0'; i++){
        if (type_string[i] != '[') { continue; }
        for (int j = i + 1; type_string[j] != '\0'; j++){
            if (type_string[j] != ']') { continue; }
            return 1;
        }
    }

    return 0;
}

int isStringPtrType(char *type_string)
{
    if (isStringArrayType(type_string)) { return 0; }

    return (strncmp(type_string, "ptr", 3) == 0);
}

int isStringStringType(char *type_string)
{
    return (strcmp(type_string, "str") == 0);
}

int isStringNumberType(char *type_string)
{
    return (isStrUnsignedInt(type_string) || isStrSignedInt(type_string));
}

int isTypeUnsignedInt(Type *type)
{
    switch(type->kind)
    {
        case TYPE_NUMBER:{
            return type->numberType.is_unsigned;
        }
        default: {
            // fprintf(stderr, "Can't calculate the signedness of a type thats not a number: '%s'\n", typeToString(type));
            // exit(1);
            return 1;
        }
    }
}


int isTypeOfKind(Type *type, TypeKind kind){
    return type->kind == kind;
}

int __areTypesCanonicallyEqual(Type *typeA, Type *typeB)
{
    if (typeA == NULL || typeB == NULL){
        fprintf(stderr, "One of the two types to compare is NULL \n");
        exit(1);
    }


    //printf("Total: %i. Checking whether typeA: %s (%p) == typeB: %s (%p)\n", gb_typeTable->count, typeToString(typeA), typeA, typeToString(typeB), typeB);
    /**
     * Different kind => Different types
     */
    if (typeA->kind != typeB->kind){
         return 0;
    }

    switch(typeA->kind)
    {
        case TYPE_NUMBER:
        {
            return ( (typeA->numberType.bytes == typeB->numberType.bytes) && (typeA->numberType.is_unsigned == typeB->numberType.is_unsigned) && 
                    (typeA->numberType.kind == typeB->numberType.kind));
        }

        case TYPE_VOID:{
            return 1;
        }

        case TYPE_PTR:{
            return __areTypesCanonicallyEqual(typeA->pointerType.pointed_type, typeB->pointerType.pointed_type);
        }

        case TYPE_ARRAY:{
            if (typeA->arrayType.n_of_elements != typeB->arrayType.n_of_elements){
                return 0;
            }
            return __areTypesCanonicallyEqual(typeA->arrayType.element_type, typeB->arrayType.element_type);
        }

        case TYPE_UDT:{
            if (typeA->udt.udt_kind != typeB->udt.udt_kind){
                return 0;
            }
        
            return (strcmp(typeA->udt.name, typeB->udt.name) == 0);
        }

        default:{
            fprintf(stderr, "Unknown kind of type to compare: %i\n", typeA->kind);
            exit(1);
        }
    } 

    //return typeA == typeB;
}

int areTypesEqual(Type *typeA, Type *typeB)
{
    // if (typeA == NULL || typeB == NULL){
    //     fprintf(stderr, "One of the two types to compare is NULL \n");
    //     exit(1);
    // }

    //printf("Checking if typeA: '%s'(%p) == typeB: '%s'(%p) --> %i\n", typeToString(typeA), typeA, typeToString(typeB), typeB, typeA == typeB);
    
    return typeA == typeB;

    // return __areTypesCanonicallyEqual(typeA, typeB);
}

/**
 * Function to try and resolve type_string from tokens and does NOT raise errors
 */
char *tryResolveTokenType(Token **tokens, int *token_pos)
{
    char *typeBuffer = malloc(256);
    memset(typeBuffer, 0, 256);


    // If it's built-in type | identifier
    if (tokens[*token_pos]->tk_type == TOKEN_TYPE || tokens[*token_pos]->tk_type == TOKEN_IDENTIFIER)
    {
        if (strcmp(tokens[*token_pos]->tk_value,"ptr") == 0)
        {
            strcat(typeBuffer, "ptr ");
            (*token_pos)++;
            demand_token(tokens, token_pos, TOKEN_OPERATOR, "<");
            // Handle pointer type recursively
            char *inner_type = tryResolveTokenType(tokens, token_pos);
            strcat(typeBuffer, inner_type);
            demand_token(tokens, token_pos, TOKEN_OPERATOR, ">");
        }

        else if (tokens[*token_pos]->tk_type == TOKEN_TYPE || tokens[*token_pos]->tk_type == TOKEN_IDENTIFIER)
        {
            strcat(typeBuffer, tokens[*token_pos]->tk_value);
            (*token_pos)++;
        }

        // At this point we can have '['
        if (tokens[*token_pos]->tk_type == TOKEN_SYMBOL && strcmp(tokens[*token_pos]->tk_value, "[") == 0)
        {
            strcat(typeBuffer, "[");
            // Advance past "["
            (*token_pos)++;

            // We could have: type[] || type[size], with size being an immediate number. Else is invalid

            if (tokens[*token_pos]->tk_type == TOKEN_NUMBER){
                strcat(typeBuffer, tokens[*token_pos]->tk_value);
                (*token_pos)++;
                demand_token(tokens, token_pos, TOKEN_SYMBOL, "]");
                strcat(typeBuffer, "]");
            }
            else if (strcmp(tokens[*token_pos]->tk_value, "]") == 0){
                strcat(typeBuffer, "]");
                demand_token(tokens, token_pos, TOKEN_SYMBOL, "]");
            }

            else {
                return NULL;
            }

        }
        typeBuffer = realloc(typeBuffer, sizeof(strlen(typeBuffer) + 1));
        return typeBuffer;
    }

    else {
        return NULL;
    }
}


char *updateTypeRepresentation(Type *t)
{
    if (t == NULL){
        printf("Can't get type representation for a null type\n");
        exit(1);
    }
    switch(t->kind)
    {
        case TYPE_VOID:
        case TYPE_STR:
        case TYPE_NUMBER:{
            return t->representation;
        }
        case TYPE_ARRAY:{
            char *element_representation = t->arrayType.element_type->representation;
            // printf("element repr of array, whose element it at: %p, is = %s\n", t->arrayType.element_type, element_representation);
            int len = strlen(element_representation) + 32;
            char *buffer = malloc(len);
            memset(buffer, 0, len);

            sprintf(buffer, "%s[%i]",element_representation, t->arrayType.n_of_elements);
            // Free old representation
            if (t->representation != NULL && strcmp(t->representation, "(unknown yet)") != 0){
                //free(t->representation);
            }
            return buffer;
        }
        case TYPE_PTR:{
            char *pointed_repr = t->pointerType.pointed_type->representation;
            
            int len = strlen(pointed_repr) + 6;
            char *buffer = malloc(len);
            memset(buffer, 0, len);
            
            sprintf(buffer, "ptr<%s>", pointed_repr);
            
            // Free old representation
            if (t->representation != NULL &&
                strcmp(t->representation, "(unknown yet)") != 0) {
                //free(t->representation);
            }
            return buffer;
        }
        case TYPE_UDT:{
            // Free the old one
            //free(t->representation);
            char *buffer = malloc(MAX_IDENTIFIER_LENGTH + 32);
            memset(buffer, 0, MAX_IDENTIFIER_LENGTH + 32);
            switch(t->udt.udt_kind)
            {
                case UDT_CLASS:{
                    strcat(buffer, "class ");
                    break;
                }
                case UDT_ENUM:{
                    strcat(buffer, "enum ");
                    break;
                }
                case UDT_OBJECT:{
                    strcat(buffer, "object ");
                    break;
                }
                case UDT_UNKNOWN_YET:{
                    strcat(buffer, "(unkown yet) ");
                    break;
                }
            }
            strcat(buffer, t->udt.name);
            return buffer;
        }
        default:{
            fprintf(stderr, "Unknown type kind to update representation of. Got: %i\n", t->kind);
            exit(1);
        }
    }
}
// Parses a type out of tokens and adds recursively to the table
Type *parseType(Token **tokens, int *token_pos)
{
    // printf("Parsing type, current token = %s\n", tokens[*token_pos]->tk_value);

    // Type := (built-in | identifier | "ptr" "<" Type ">") ("[" (number)? "]")
    Type *type_so_far = NULL;

    if (strcmp(tokens[*token_pos]->tk_value,"ptr") == 0){
        (*token_pos)++;
        demand_token(tokens, token_pos, TOKEN_OPERATOR, "<");
        Type *inner_type = parseType(tokens, token_pos);
        if (inner_type == NULL){
            return NULL;
        }

        Type *ptr_type = makePointerType(inner_type);
        ptr_type->representation = updateTypeRepresentation(ptr_type);

        type_so_far = __addTypeToTable(ptr_type);
        // printf("Made a pointer type , with inner at %p\n", inner_type);
        demand_token(tokens, token_pos, TOKEN_OPERATOR, ">");
    }
    else if (tokens[*token_pos]->tk_type == TOKEN_TYPE){
        // Already in table, find it
        Type *builtin = findBuiltinInTable(tokens[*token_pos]->tk_value);
        if (builtin == NULL){
            printf("Failed to find builtin type '%s' when parsing type\n", tokens[*token_pos]->tk_value);
            exit(1);
        }
        (*token_pos)++;

        type_so_far = builtin;
        // printf("Here\n");
    }
    else if (tokens[*token_pos]->tk_type == TOKEN_IDENTIFIER){
        Type *udt = makeUDTType(tokens[*token_pos]->tk_value);
        udt->representation = updateTypeRepresentation(udt);
        (*token_pos)++;

        type_so_far = __addTypeToTable(udt);
    }


    // If null at this point, return null
    if (type_so_far == NULL){
        return NULL;
    }


    if (strcmp(tokens[*token_pos]->tk_value, "[") == 0){
        (*token_pos)++;

        Type *array_type;
        // At this point, before making the array, be sure to use a base type cached if possible
        if (tokens[*token_pos]->tk_type == TOKEN_NUMBER){
            int n_elements = atoi(tokens[*token_pos]->tk_value);
            array_type = makeArrayType(type_so_far, ARR_SIZE_GIVEN, n_elements);
            (*token_pos)++;
        }
        else if (strcmp(tokens[*token_pos]->tk_value, "]") == 0){
            array_type = makeArrayType(type_so_far, ARR_SIZE_INTERPRETED, 0);
        }
        else{
            // Error
            return NULL;
        }
        demand_token(tokens, token_pos, TOKEN_SYMBOL, "]");
        // printf("Before adding the array, base type representation = %s\n", type_so_far->representation);
        array_type->representation = updateTypeRepresentation(array_type);
        // printf("Array type representation = %s\n", array_type->representation);
        type_so_far = __addTypeToTable(array_type);
    }

    // type_so_far->representation = "(unknown yet)";
    // type_so_far->representation = updateTypeRepresentation(type_so_far);
    // Type *new_or_cached = __addTypeToTable(type_so_far);
    
    return type_so_far;
}

// Tries to parse a type out of tokens and DOES NOT add to the table
Type *tryParseType(Token **tokens, int *token_pos)
{
    // printf("Trying parsing type, current token = %s\n", tokens[*token_pos]->tk_value);

    // Type := (built-in | identifier | "ptr" "<" Type ">") ("[" (number)? "]")
    Type *type_so_far = NULL;

    if (strcmp(tokens[*token_pos]->tk_value,"ptr") == 0){
        (*token_pos)++;
        if(!expect_token(tokens, token_pos, TOKEN_OPERATOR, "<")){
            return NULL;
        }
        Type *inner_type = parseType(tokens, token_pos);
        if (inner_type == NULL){
            return NULL;
        }

        Type *ptr_type = makePointerType(inner_type);
        // ptr_type->representation = updateTypeRepresentation(ptr_type);

        type_so_far = ptr_type;
        // printf("Made a pointer type , with inner at %p\n", inner_type);
        if (!expect_token(tokens, token_pos, TOKEN_OPERATOR, ">")){
            return NULL;
        }
    }
    else if (tokens[*token_pos]->tk_type == TOKEN_TYPE){
        // Already in table, find it
        Type *builtin = findBuiltinInTable(tokens[*token_pos]->tk_value);
        if (builtin == NULL){
            printf("Failed to find builtin type '%s' when parsing type\n", tokens[*token_pos]->tk_value);
            exit(1);
        }
        (*token_pos)++;
        // printf("Current token after built in type = %s\n", tokens[*token_pos]->tk_value);
        type_so_far = builtin;
        // printf("Here\n");
    }
    else if (tokens[*token_pos]->tk_type == TOKEN_IDENTIFIER){
        Type *udt = makeUDTType(tokens[*token_pos]->tk_value);
        // udt->representation = updateTypeRepresentation(udt);
        (*token_pos)++;

        type_so_far = udt;
    }


    // If null at this point, return null
    if (type_so_far == NULL){
        return NULL;
    }


    if (strcmp(tokens[*token_pos]->tk_value, "[") == 0){
        (*token_pos)++;
        // printf("Current token after array start = %s\n", tokens[*token_pos]->tk_value);

        Type *array_type;
        // At this point, before making the array, be sure to use a base type cached if possible
        if (tokens[*token_pos]->tk_type == TOKEN_NUMBER){
            int n_elements = atoi(tokens[*token_pos]->tk_value);
            array_type = makeArrayType(type_so_far, ARR_SIZE_GIVEN, n_elements);
            (*token_pos)++;
        }
        else if (strcmp(tokens[*token_pos]->tk_value, "]") == 0){
            array_type = makeArrayType(type_so_far, ARR_SIZE_INTERPRETED, 0);
        }
        else{
            // Error
            return NULL;
        }
        // printf("Current token = %s\n", tokens[*token_pos]->tk_value);
        if (!expect_token(tokens, token_pos, TOKEN_SYMBOL, "]")){
            return NULL;
        }
        // printf("Before adding the array, base type representation = %s\n", type_so_far->representation);
        // array_type->representation = updateTypeRepresentation(array_type);
        // printf("Array type representation = %s\n", array_type->representation);
        type_so_far = array_type;
    }

    // type_so_far->representation = "(unknown yet)";
    // type_so_far->representation = updateTypeRepresentation(type_so_far);
    // Type *new_or_cached = __addTypeToTable(type_so_far);
    
    return type_so_far;
}


void __initTypeTable()
{
    gb_typeTable = malloc(sizeof(TypeTable));
    gb_typeTable->types = malloc(TYPETABLE_INIT_CAPACITY * sizeof(Type *));
    gb_typeTable->capacity = TYPETABLE_INIT_CAPACITY;
    gb_typeTable->count = 0;
    gb_typeTable->undef_count = 0;

    addBuiltInTypes();
}

void __resizeTypeTable()
{
    gb_typeTable->capacity = TYPETABLE_RESIZE_FACTOR * gb_typeTable->capacity;
    gb_typeTable->types = realloc(gb_typeTable->types, gb_typeTable->capacity * sizeof(Type *));
}


void addBuiltInTypes()
{
    Type *void_type = makeVoidType();
    void_type->representation = "void";
    __addTypeToTable(void_type);
    // UINT AND INT
    Type *uint8 = makeNumberType(1, 1, NUMBER_INTEGER);
    uint8->representation = strdup("uint8");
    __addTypeToTable(uint8);
    Type *uint16 = makeNumberType(2, 1, NUMBER_INTEGER);
    uint16->representation = strdup("uint16");
    __addTypeToTable(uint16);
    Type *uint32 = makeNumberType(4, 1, NUMBER_INTEGER);
    uint32->representation = strdup("uint32");
    __addTypeToTable(uint32);
    Type *uint64 = makeNumberType(8, 1, NUMBER_INTEGER);
    uint64->representation = strdup("uint64");
    __addTypeToTable(uint64);
    Type *int8 = makeNumberType(1, 0 ,NUMBER_INTEGER);
    int8->representation = strdup("int8");
    __addTypeToTable(int8);
    Type *int16 = makeNumberType(2, 0 , NUMBER_INTEGER);
    int16->representation = strdup("int16");
    __addTypeToTable(int16);
    Type *int32 = makeNumberType(4, 0, NUMBER_INTEGER);
    int32->representation = strdup("int32");
    __addTypeToTable(int32);
    Type *int64 = makeNumberType(8, 0, NUMBER_INTEGER);
    int64->representation = strdup("int64");
    __addTypeToTable(int64);


    // BOOL, CHAR, STR 
    Type *b = makeNumberType(1, 1, NUMBER_BOOL);
    b->representation = strdup("bool");
    __addTypeToTable(b);
    Type *c = makeNumberType(1, 1, NUMBER_CHAR);
    c->representation = strdup("char");
    __addTypeToTable(c);
    Type *str = makePointerType(c);
    str->representation = strdup("ptr<char>");
    __addTypeToTable(str);

}

Type *findBuiltinInTable(char *repr)
{
    if (strcmp(repr, "str") == 0){
        repr = "ptr<char>";
    }
    for (int i = 0; i < gb_typeTable->count; i++){
        if (strcmp(gb_typeTable->types[i]->representation, repr) == 0){
            return gb_typeTable->types[i];
        }
    }

    return NULL;
}

Type *findUDTInTable(char *name)
{
    for (int i = 0; i < gb_typeTable->count; i++){
        if (gb_typeTable->types[i]->kind != TYPE_UDT){
            continue;
        }

        if (strcmp(gb_typeTable->types[i]->udt.name, name) == 0){
            return gb_typeTable->types[i];
        }
    }

    return NULL;
}

int isTypeInTable(Type *type)
{
    for (int i = 0; i < gb_typeTable->count; i++){
        // if (__areTypesCanonicallyEqual(gb_typeTable->types[i], type)){
        // if (gb_typeTable->types[i] == type){
        if (areTypesEqual(type, gb_typeTable->types[i])){
            return 1;
        }
    }

    return 0;
}

// Type has to be on the table
int isValidSizeofType(Type *type)
{
    for (int i = 0; i < gb_typeTable->count; i++){
        if (__areTypesCanonicallyEqual(type, gb_typeTable->types[i])){
            return 1;
        }
    }
    return 0;
}

// Type base has to be on the table
int isValidCastType(Type *type)
{
    Type *base = getBaseType(type);
    return isTypeInTable(base);

}

char *typeToString(Type *type)
{
    return type->representation;
}

Type *__addTypeToTable(Type *type)
{
    // printf("Trying to add a type : %s \n", type->representation);
    /**
     * First check whether the type is already in the table, if so, free the new instance and return the one in the table
     */
    for (int i = 0; i < gb_typeTable->count; i++){

        // 1/17 BUG: TYPE_STR == TYPE_PTR CHAR but then every single ptr<char> after that is str
        // they should be two separate instances in the type table
        // Fixed with canonically equal
        if (__areTypesCanonicallyEqual(type, gb_typeTable->types[i])){
        // if (type == gb_typeTable->types[i]){
            // printf("Type was already in table, we are freeing\n");
            // free(type);
            //printf("Returning cached type: %s\n", typeToString(gb_typeTable->types[i]));
            return gb_typeTable->types[i];
        }
    }
    

    //printf("We added a new type.\n");

    /**
     * We have a new type
     */
    if (gb_typeTable->count + 1 >= gb_typeTable->capacity){
        __resizeTypeTable();
    }

    gb_typeTable->types[gb_typeTable->count++] = type;
    // printf("Added type: %s, to table\n", typeToString(type));

    // printTypeTable();

    return type;
}

void printTypeTable()
{
    printf("====================================================================\n");
    printf("%-16s %-50s %-30s\n", "Index", "Type", "Address");
    printf("--------------------------------------------------------------------\n");
    for (int i = 0; i < gb_typeTable->count; i++){
        printf("%-16i %-50s %-30p\n", i, gb_typeTable->types[i]->representation, gb_typeTable->types[i]);
    }
}


Type *__allocateType()
{
    Type *type = malloc(sizeof(Type));
    memset(type, 0, sizeof(Type));
    return type;
}

Type *makeVoidType()
{
    Type *void_type = __allocateType();
    void_type->kind = TYPE_VOID;


    //return __addTypeToTable(void_type);
    return void_type;
}

Type *makeNumberType(int bytes, int is_unsigned, NumberKind kind)
{
    Type *number_type = __allocateType();
    number_type->kind = TYPE_NUMBER;
    number_type->numberType.bytes = bytes;
    number_type->numberType.is_unsigned = is_unsigned;
    number_type->numberType.kind = kind;


    //return __addTypeToTable(number_type);
    return number_type;
}

Type *makePointerType(Type *pointed_type)
{
    Type *pointer_type = __allocateType();
    pointer_type->kind = TYPE_PTR;

    pointer_type->pointerType.pointed_type = pointed_type;

    return pointer_type;
}

Type *makeArrayType(Type *el_type, ArraySizeKind size_kind, int n_elements)
{
    Type *array_type = __allocateType();
    array_type->kind = TYPE_ARRAY;
    array_type->arrayType.element_type = el_type;
    array_type->arrayType.size_provided_kind = size_kind;
    array_type->arrayType.n_of_elements = n_elements;

    return array_type;
}

Type *makeUDTType(char *type_string)
{

    Type *udt_type = __allocateType();
    udt_type->kind = TYPE_UDT;
    udt_type->udt.name = strdup(type_string);
    udt_type->udt.udt_kind = UDT_UNKNOWN_YET;

    return udt_type;
}

Type *findNumberTypeInTable(int bytes, int is_unsigned, NumberKind kind)
{
    for (int i = 0; i < gb_typeTable->count; i++){
        Type *current = gb_typeTable->types[i];
        if (current->kind != TYPE_NUMBER){
            continue;
        }
        if (current->numberType.bytes == bytes && current->numberType.is_unsigned == is_unsigned && current->numberType.kind == kind){
            return current;
        }
    }

    return NULL;
}

Type *getBaseType(Type *t)
{
    switch(t->kind)
    {
        case TYPE_PTR:{
            return getFinalPointedType(t);
        }
        case TYPE_ARRAY:{
            return t->arrayType.element_type;
        }
        default:{
            return t;
        }
    }
    return t;
}

void trackUndefinedTypes(Type *t)
{
    Type *base = getBaseType(t);

    if (base->kind == TYPE_UDT){
        gb_typeTable->undef_count++;
    }
}

Type *resolveNumberType(uint64_t n, int is_unsigned)
{
    int bytes = 8;

    if (!is_unsigned){
        if (0 <= n && n <= INT8_MAX){
            bytes = 1;
        }
        else if (INT8_MAX < n && n <= INT16_MAX){
            bytes = 2;
        }
        else if (INT16_MAX < n && n <= INT32_MAX){
            bytes = 4;
        }
        else if (INT32_MAX < n && n <= INT64_MAX){
            bytes = 8;
        }
        else {
            Error *e = invoke(NUMBER_TOO_BIG);
            appendMessageToError(e, fetchErrorMsgBasedOnErrorID(e->error_id));
            addError(e);
        }
    }
    else{
        if (0 <= n && n <= UINT8_MAX){
            bytes = 1;
        }
        else if (UINT8_MAX < n && n <= UINT16_MAX){
            bytes = 2;
        }
        else if (UINT16_MAX < n && n <= UINT32_MAX){
            bytes = 4;
        }
        else if (UINT32_MAX < n && n <= UINT64_MAX){
            bytes = 8;
        }
        else {
            Error *e = invoke(NUMBER_TOO_BIG);
            appendMessageToError(e, fetchErrorMsgBasedOnErrorID(e->error_id));
            addError(e);
        }
    }

    Type *number_type = makeNumberType(bytes, is_unsigned, NUMBER_INTEGER);

    return __addTypeToTable(number_type);
}


/**
 * Dynamically create types, this is during analysis.
 */

Type *createNumberTypeFromInfo(int bytes, int is_unsigned, NumberKind nkind)
{
    Type *t = __allocateType();
    t->kind = TYPE_NUMBER;
    t->numberType.bytes = bytes;
    t->numberType.is_unsigned = is_unsigned;
    t->numberType.kind = nkind;

    return __addTypeToTable(t);
}

Type *createPointerTypeFromInfo(Type *pointed_type)
{
    Type *t = __allocateType();
    t->kind = TYPE_PTR;
    t->pointerType.pointed_type = pointed_type;


    return __addTypeToTable(t);
}

Type *createArrayTypeFromInfo(int n_elements, Type *element_type)
{
    Type *t = __allocateType();
    t->kind = TYPE_ARRAY;
    t->arrayType.element_type = element_type;
    t->arrayType.size_provided_kind = ARR_SIZE_INTERPRETED;
    t->arrayType.n_of_elements = n_elements;

    return __addTypeToTable(t);
}


/**
 * ===============================================================================
 * END TYPES OPERATIONS     
 * ===============================================================================   
 */




/**
 * ===============================================================================
 * AST MAKERS     
 * ===============================================================================   
 */
Parameter *parseFunctionParameter(Token **tokens, int *token_pos)
{
    Parameter *p = malloc(sizeof(Parameter));

    // printf("Parsing func parameter with type: '%s'\n", tokens[*token_pos]->tk_value);

    if (strcmp(tokens[*token_pos]->tk_value, "varargs") == 0){
        p->ptype = P_VARARGS;
        (*token_pos)++;
    }
    else{
        p->type = parseType(tokens, token_pos);
        //printTypeTable();
        // printf("Parsed a parameter type, sitting at %p\n", p->type);
        char *id = tokens[*token_pos]->tk_value;
        demand_token_type(tokens, token_pos, TOKEN_IDENTIFIER);

        p->name = id;
        p->ptype = P_INMEDIATE;
    }

    //printf("Finished parsing func paramm\n");

    return p;
}

char *mystrdup(const char *src)
{
    if (src == NULL) { return NULL; }
    return strdup(src);
}


ASTNode *__allocateAST()
{
    // ASTNode *ast = arena_alloc(sizeof(ASTNode), POOL_AST);
    ASTNode *ast = malloc(sizeof(ASTNode));
    memset(ast, 0, sizeof(ASTNode));

    // ast_count++;

    return ast;
}

ASTNode *makeIdentifier(char *identifier)
{
    ASTNode *ast_id = __allocateAST();

    ast_id->node_type = NODE_IDENTIFIER;
    ast_id->identifier_node.name = mystrdup(identifier);
    //ast_id->identifier_node.line_number = tracker.current_line;
    return ast_id;
}

ASTNode *makeAssignment(Type *type, char *identifier, ASTNode *expression)
{
    ASTNode *ast_assignment = __allocateAST();

    ast_assignment->node_type = NODE_ASSIGNMENT;
    ast_assignment->assignment_node.type = type;
    ast_assignment->assignment_node.identifier = mystrdup(identifier);
    ast_assignment->assignment_node.expression = expression;

    return ast_assignment;
}

ASTNode *makeReassignment(ASTNode *lvalue, TokenType op, ASTNode *expression, Type *type)
{
    ASTNode *ast_reassignment = __allocateAST();

    ast_reassignment->node_type = NODE_REASSIGNMENT;
    ast_reassignment->reassignment_node.lvalue = lvalue;
    ast_reassignment->reassignment_node.expression = expression;
    ast_reassignment->reassignment_node.op = op;
    ast_reassignment->reassignment_node.type = type;

    return ast_reassignment;
}

ASTNode *makeDeclaration(Type *type, char *identifier)
{
    ASTNode *ast_decl = __allocateAST();

    ast_decl->node_type = NODE_DECLARATION;
    ast_decl->declaration_node.identifier = mystrdup(identifier);
    ast_decl->declaration_node.type = type;

    return ast_decl;
}

ASTNode *makePostFix(ASTNode *left, Type *type, int size_of_operand, char *op)
{
    ASTNode *ast_stdalonePF = __allocateAST();

    ast_stdalonePF->node_type = NODE_POSTFIX_OP;
    ast_stdalonePF->postfix_op_node.left = left;
    ast_stdalonePF->postfix_op_node.type = type;
    ast_stdalonePF->postfix_op_node.size_of_operand = size_of_operand;
    ast_stdalonePF->postfix_op_node.op = mystrdup(op);

    return ast_stdalonePF;
}

ASTNode *makeSubscript(ASTNode *base, ASTNode *index, Type *base_type, int element_size, int index_size)
{
    //printf("Here \n");
    ASTNode *ast_subscript = __allocateAST();

    ast_subscript->node_type = NODE_SUBSCRIPT;
    ast_subscript->subscript_node.base = base;
    ast_subscript->subscript_node.index = index;
    ast_subscript->subscript_node.base_type = base_type;
    ast_subscript->subscript_node.element_size = element_size;
    ast_subscript->subscript_node.index_size = index_size;

    return ast_subscript;
}

ASTNode *makeWhile(ASTNode *condition_expr, ASTNode *body)
{
    ASTNode *ast_while = __allocateAST();

    ast_while->node_type = NODE_WHILE;
    ast_while->while_node.condition_expr = condition_expr;
    ast_while->while_node.body = body;

    return ast_while;
}


ASTNode *makeFor(ASTNode *assignment_expr, ASTNode *condition_expr, ASTNode *reassignment_expr, ASTNode *body)
{
    ASTNode *ast_for = __allocateAST();
    ast_for->node_type = NODE_FOR;
    ast_for->for_node.assignment_expr = assignment_expr;
    ast_for->for_node.condition_expr = condition_expr;
    ast_for->for_node.reassignment_expr = reassignment_expr;
    ast_for->for_node.body = body;
    ast_for->for_node.label = NULL;

    return ast_for;
}

ASTNode *makeFuncCall(char *identifier, int params_count, ASTNode **params_expr, Type **params_type)
{
    ASTNode *ast_funccall = __allocateAST();

    ast_funccall->node_type = NODE_FUNC_CALL;
    ast_funccall->funccall_node.identifier = mystrdup(identifier);
    ast_funccall->funccall_node.params_count = params_count;
    ast_funccall->funccall_node.params_expr = params_expr;
    ast_funccall->funccall_node.params_type = params_type;

    return ast_funccall;
}

ASTNode *makeBinOP(ASTNode *left, ASTNode *right, char *op)
{
    ASTNode *ast_binop = __allocateAST();

    ast_binop->node_type = NODE_BINARY_OP;
    ast_binop->binary_op_node.left = left;
    ast_binop->binary_op_node.right = right;
    ast_binop->binary_op_node.op = mystrdup(op);

    return ast_binop;
}

ASTNode *makeUnaryOP(ASTNode *right, char *op, int size_of_operand)
{
    ASTNode *ast_unaryop = __allocateAST();

    ast_unaryop->node_type = NODE_UNARY_OP;
    ast_unaryop->unary_op_node.op = mystrdup(op);
    ast_unaryop->unary_op_node.right = right;
    ast_unaryop->unary_op_node.size_of_operand = size_of_operand;

    return ast_unaryop;
}

ASTNode *makeBlock(ASTNode **statements, int count, int capacity)
{
    ASTNode *ast_block = __allocateAST();

    ast_block->node_type = NODE_BLOCK;
    ast_block->block_node.capacity = capacity;
    ast_block->block_node.statement_count = count;
    ast_block->block_node.statements = statements;

    return ast_block;
}




/**
 * ===============================================================================
 * END AST MAKERS     
 * ===============================================================================   
 */



/**
 * ===============================================================================
 * AST PARSING   
 * ===============================================================================   
 */

void __resizeStmtBlock(ASTNode *stmt_block)
{
    if (stmt_block->node_type != NODE_BLOCK){
        fprintf(stderr, "Cant resize an AST statment block whose node type is not NODE_BLOCK\n");
        exit(1);
    }

    stmt_block->block_node.capacity *= STMTBLOCK_RESIZE_FACTOR;
    stmt_block->block_node.statements = realloc(stmt_block->block_node.statements, stmt_block->block_node.capacity * sizeof(ASTNode *));

}

ASTNode *parseAssignment(Token **tokens, int *token_pos)
{

    // At this point we have an identifier | type 
    ASTNode *ast_assignment = malloc(sizeof(ASTNode));
    ast_assignment->node_type = NODE_ASSIGNMENT;
    ast_assignment->assignment_node.line_number = tokens[*token_pos]->line_number;

    ast_assignment->assignment_node.type = parseType(tokens, token_pos);


    // At this point we have an identifier for the variable 
    ast_assignment->assignment_node.identifier = tokens[*token_pos]->tk_value;
    ast_assignment->assignment_node.line_number = tokens[*token_pos]->line_number;
    (*token_pos)++;
    
    demand_token(tokens, token_pos, TOKEN_ASSIGN, "=");

    // Special case. Expression begins with '[' -> array init 
    if (strcmp(tokens[*token_pos]->tk_value, "[") == 0)
    {
        (*token_pos)++;
        ast_assignment->assignment_node.expression = parseArrayInit(tokens, token_pos, ast_assignment->assignment_node.identifier, ast_assignment->assignment_node.type);
        demand_token(tokens, token_pos, TOKEN_SYMBOL, "]");
        return ast_assignment;
    }


    ast_assignment->assignment_node.expression = parseExpression(tokens, token_pos);


    return ast_assignment;
    

}

ASTNode *parseProgram(Token **tokens, int *token_pos)
{

    ASTNode *program = __allocateAST();

    program->node_type = NODE_BLOCK;
    program->block_node.statements = malloc(INIT_BLOCK_STMT_CAPACITY * sizeof(ASTNode *));
    program->block_node.line_number = tokens[*token_pos]->line_number;
    program->block_node.statement_count = 0;
    program->block_node.capacity = INIT_BLOCK_STMT_CAPACITY;


    while (tokens[*token_pos]->tk_type != TOKEN_EOF)
    {
        if (program->block_node.statement_count + 1 >= program->block_node.capacity){
            __resizeStmtBlock(program);
        }
        // printf("Parsing stmt #%i\n", program->block_node.statement_count);
        program->block_node.statements[program->block_node.statement_count++] = parseStatement(tokens, token_pos);
    }


    return program;
}

ASTNode *parseBlock(Token **tokens, int *token_pos)
{
    // Create the block node
    ASTNode *block = __allocateAST();
    block->node_type = NODE_BLOCK;
    block->block_node.line_number = tokens[*token_pos]->line_number;
    block->block_node.statements = malloc(INIT_BLOCK_STMT_CAPACITY * sizeof(ASTNode *));
    block->block_node.statement_count = 0;
    block->block_node.capacity = INIT_BLOCK_STMT_CAPACITY; // Initial capacity

    // Parse statements until the end of the block or EOF
    while (tokens[*token_pos]->tk_type != TOKEN_EOF && (strcmp(tokens[*token_pos]->tk_value, "}") != 0) )
    {
        // Resize the array if necessary
        if (block->block_node.statement_count + 1 >= block->block_node.capacity){
            __resizeStmtBlock(block);
        }
        // Parse the next statement and add it to the block
        ASTNode *stmt = parseStatement(tokens, token_pos);
        block->block_node.statements[block->block_node.statement_count++] = stmt;
    }

    return block;
}

ASTNode *parseClassDef(Token **tokens, int *token_pos)
{
    // printf("Parsing class def\n");
    ASTNode *ast_class = __allocateAST();
    ast_class->node_type = NODE_CLASS;
    ast_class->class_node.line_number = tokens[*token_pos]->line_number;
    ast_class->class_node.parent = NULL;
    ast_class->class_node.is_forward = 0;

    if (strcmp(tokens[*token_pos]->tk_value, "forward") == 0){
        ast_class->class_node.is_forward = 1;
        (*token_pos)++;
    }
    // Move past the 'class' keyword
    (*token_pos)++;
    ast_class->class_node.identifier = tokens[*token_pos]->tk_value;
    // Parse the type
    Type *class_type = parseType(tokens, token_pos);
    // class_type->udt.udt_kind = UDT_CLASS;
    // class_type->representation = updateTypeRepresentation(class_type);
    ast_class->class_node.class_type = class_type;
    // Move past the identifier
    //(*token_pos)++;
    if (expect_token(tokens, token_pos, TOKEN_KEYWORD, "extends")){
        // Get the parent 
        ast_class->class_node.parent = tokens[*token_pos]->tk_value;
        (*token_pos)++;
    }


    if (ast_class->class_node.is_forward == 1){
        demand_token(tokens, token_pos, TOKEN_SYMBOL, ";");
        return ast_class;
    }

    demand_token(tokens, token_pos, TOKEN_SYMBOL, "{");

    ast_class->class_node.declaration_count = 0;
    ast_class->class_node.declaration_capacity = INIT_CLASS_DECL_CAPACITY;
    ast_class->class_node.declarations = malloc(sizeof(ASTNode *) * INIT_CLASS_DECL_CAPACITY);
    ast_class->class_node.funcdefs_count = 0;
    ast_class->class_node.funcdefs_capacity = INIT_CLASS_FUNCDEF_CAPACITY;
    ast_class->class_node.func_defs = malloc(sizeof(ASTNode *) * INIT_CLASS_FUNCDEF_CAPACITY);


    while (strcmp(tokens[*token_pos]->tk_value, "}") != 0)
    {
        StmtKind stmt_kind = classifyStatement(tokens, token_pos);
        // printf("In class def, current token = %s\n", tokens[*token_pos]->tk_value);

        switch(stmt_kind)
        {
            case STMT_DECLARATION:{
                if (ast_class->class_node.declaration_count + 1 >= ast_class->class_node.declaration_capacity){
                    ast_class->class_node.declaration_capacity *= CLASS_STMT_RESIZE_FACTOR;
                    ast_class->class_node.declarations = realloc(ast_class->class_node.declarations, ast_class->class_node.declaration_capacity * sizeof(ASTNode *));
                }
                ast_class->class_node.declarations[ast_class->class_node.declaration_count++] = parseDeclaration(tokens, token_pos);
                demand_token(tokens, token_pos, TOKEN_SYMBOL, ";");
                break;
            }

            case STMT_UNION:{
                if (ast_class->class_node.declaration_count + 1 >= ast_class->class_node.declaration_capacity){
                    ast_class->class_node.declaration_capacity *= CLASS_STMT_RESIZE_FACTOR;
                    ast_class->class_node.declarations = realloc(ast_class->class_node.declarations, ast_class->class_node.declaration_capacity * sizeof(ASTNode *));
                }
                ast_class->class_node.declarations[ast_class->class_node.declaration_count++] = parseUnion(tokens, token_pos);
                demand_token(tokens, token_pos, TOKEN_SYMBOL, ";");
                break;
            }

            case STMT_FUNC_DEF:{
                if (ast_class->class_node.funcdefs_count + 1 >= ast_class->class_node.funcdefs_capacity){
                    ast_class->class_node.funcdefs_capacity *= CLASS_STMT_RESIZE_FACTOR;
                    ast_class->class_node.func_defs = realloc(ast_class->class_node.func_defs, ast_class->class_node.funcdefs_capacity * sizeof(ASTNode *));
                }
                // printf("Parsing CLASS func def\n");
                ast_class->class_node.func_defs[ast_class->class_node.funcdefs_count++] = parseFuncDef(tokens, token_pos);
                break;
            }
            case STMT_EMPTY:{
                expect_token(tokens, token_pos, TOKEN_SYMBOL, ";");
                break;
            }
            default:{
                printf("In file: '%s'. In L = %i. Unexpected statement in class def, only declarations, unions and functions are allowed, but got: '%s'.\n", 
                    tracker.current_src_file, tracker.current_line, statementToStr(stmt_kind));
                exit(1);
                break;
            }
        }
    }


    demand_token(tokens, token_pos, TOKEN_SYMBOL, "}");

    return ast_class;

}

ASTNode *parseEnumDef(Token **tokens, int *token_pos)
{
    ASTNode *ast_enum = __allocateAST();
    ast_enum->node_type = NODE_ENUM;
    ast_enum->enum_node.line_number = tokens[*token_pos]->line_number;
    ast_enum->enum_node.is_forward = 0;

    if(strcmp(tokens[*token_pos]->tk_value, "forward") == 0){
        ast_enum->enum_node.is_forward = 1;
        (*token_pos)++;
    }
    // past enum
    (*token_pos)++;

    //Get the identifier
    ast_enum->enum_node.identifier = tokens[*token_pos]->tk_value;
    Type *enum_type = parseType(tokens, token_pos);
    // enum_type->udt.udt_kind = UDT_ENUM;
    // enum_type->representation = updateTypeRepresentation(enum_type);
    ast_enum->enum_node.enum_type = enum_type;

    // By default, base type = uint8 
    ast_enum->enum_node.base_type = createNumberTypeFromInfo(1, 1, NUMBER_INTEGER);

    if (expect_token(tokens, token_pos, TOKEN_SYMBOL, ":")){
        ast_enum->enum_node.base_type = parseType(tokens, token_pos);
    }
    // Create the type out of that
    // Type *enum_type = parseType(tokens, token_pos);


    if (ast_enum->enum_node.is_forward == 1){
        demand_token(tokens, token_pos, TOKEN_SYMBOL, ";");
        return ast_enum;
    }

    ast_enum->enum_node.declaration_count = 0;
    ast_enum->enum_node.declaration_capacity = INIT_ENUM_DECL_CAPACTITY;
    ast_enum->enum_node.declarations = malloc(INIT_ENUM_DECL_CAPACTITY * sizeof(ASTNode *));

    demand_token(tokens, token_pos, TOKEN_SYMBOL, "{");

    //Type *enum_type = makeUDTType(ast_enum->enum_node.identifier);
    // printf("Parsing enum def\n");
    while (strcmp(tokens[*token_pos]->tk_value, "}") != 0)
    {
        StmtKind stmt_kind = classifyStatement(tokens, token_pos);
        // printf("Statement classified into: %s\n", statementToStr(stmt_kind));
        switch (stmt_kind)
        {
            case STMT_EXPRESSION:{
                if (ast_enum->enum_node.declaration_count + 1 >= ast_enum->enum_node.declaration_capacity){
                    ast_enum->enum_node.declaration_capacity *= ENUM_DECL_RESIZE_FACTOR;
                    ast_enum->enum_node.declarations = realloc(ast_enum->enum_node.declarations, ast_enum->enum_node.declaration_capacity * sizeof(ASTNode *));
                }
                ast_enum->enum_node.declarations[ast_enum->enum_node.declaration_count++] = parseExpression(tokens, token_pos);
                break;
            }

            case STMT_REASSIGNMENT:{
                if (ast_enum->enum_node.declaration_count + 1 >= ast_enum->enum_node.declaration_capacity){
                    ast_enum->enum_node.declaration_capacity *= ENUM_DECL_RESIZE_FACTOR;
                    ast_enum->enum_node.declarations = realloc(ast_enum->enum_node.declarations, ast_enum->enum_node.declaration_capacity * sizeof(ASTNode *));
                }
                ast_enum->enum_node.declarations[ast_enum->enum_node.declaration_count++] = parseReassignment(tokens, token_pos);
                break;
            }
            default:{
                printf("In file: '%s'. In L = %i. Unexpected statement in enum def, only declarations and assignments are allowed, but got: '%s'.\n", 
                    tracker.current_src_file, tracker.current_line, statementToStr(stmt_kind));
                exit(1);
                break;
            }
        }
        // If its the last one, no need to have ","
        if (strcmp(tokens[*token_pos]->tk_value, "}") == 0){
            break;
        }
        // Otherwise demand the ","
        demand_token(tokens, token_pos, TOKEN_SYMBOL, ",");
    }
    
    demand_token(tokens, token_pos, TOKEN_SYMBOL, "}");
    return ast_enum;
}

ASTNode *parseEmpty(Token **tokens, int *token_pos)
{
    //Advance to next token
    (*token_pos)++;
    //Create null node and return it
    ASTNode *ast_null = __allocateAST();
    ast_null->node_type = NODE_NULL;
    return ast_null;
}



ASTNode *parseExternFuncDef(Token **tokens, int *token_pos)
{
    // Create extern func node 
    ASTNode *ast_extern_func_def = __allocateAST();
    ast_extern_func_def->node_type = NODE_EXTERN_FUNC_DEF;
    ast_extern_func_def->extern_func_def_node.line_number = tokens[*token_pos]->line_number;
    ast_extern_func_def->extern_func_def_node.params = malloc(INIT_FUNCDEF_PARAM_CAPACITY * sizeof(Parameter *));
    ast_extern_func_def->extern_func_def_node.params_count = 0;
    ast_extern_func_def->extern_func_def_node.param_capacity = INIT_FUNCDEF_PARAM_CAPACITY;
    

    // Advance past 'extern' and 'func'
    (*token_pos) += 2;

    ast_extern_func_def->extern_func_def_node.return_type = parseType(tokens, token_pos);

    ast_extern_func_def->extern_func_def_node.func_name = tokens[*token_pos]->tk_value;
    // printf("We got the function name: %s \n", ast_func_def->funcdef_node.func_name);
    // Advance past function name
    (*token_pos)++;
    // printf("Current token is: %s \n", tokens[*token_pos]->tk_value);
    demand_token(tokens, token_pos, TOKEN_SYMBOL, "(");
    // printf("Passed it \n");
    // Parameters

    while (strcmp(tokens[*token_pos]->tk_value, ")") != 0){
        if (ast_extern_func_def->extern_func_def_node.params_count + 1 >= ast_extern_func_def->extern_func_def_node.param_capacity){
            ast_extern_func_def->extern_func_def_node.param_capacity *= FUNCDEF_PARAM_RESIZE_FACTOR;
            ast_extern_func_def->extern_func_def_node.params = realloc(ast_extern_func_def->extern_func_def_node.params ,ast_extern_func_def->extern_func_def_node.param_capacity * sizeof(Parameter *));
        }
        // Parse the parameters
        ast_extern_func_def->extern_func_def_node.params[ast_extern_func_def->extern_func_def_node.params_count] = parseFunctionParameter(tokens,token_pos);
        ast_extern_func_def->extern_func_def_node.params_count++;

        expect_token(tokens, token_pos, TOKEN_SYMBOL, ",");
    }

    demand_token(tokens, token_pos, TOKEN_SYMBOL, ")");

    return ast_extern_func_def;   
}


ASTNode *parseExternId(Token **tokens, int *token_pos)
{
    // past extern
    (*token_pos)++;
    ASTNode *ext_id = __allocateAST();
    ext_id->node_type = NODE_EXTERN_IDENTIFIER;
    ext_id->extern_identifier_node.line_number = tokens[*token_pos]->line_number;
    // resolve type
    ext_id->extern_identifier_node.type = parseType(tokens, token_pos);

    ext_id->extern_identifier_node.name = strdup(tokens[*token_pos]->tk_value);
    // past name
    (*token_pos)++;
    return ext_id;
}

// Declaration  := type identifier ';'
// type = object? identifier | enum? identifier | built-in type 
ASTNode *parseDeclaration(Token **tokens,int *token_pos)
{
    // printf("Parsing declaration\n");
    ASTNode *ast_declaration = malloc(sizeof(ASTNode));
    ast_declaration->node_type = NODE_DECLARATION;
    ast_declaration->declaration_node.line_number = tokens[*token_pos]->line_number;

    ast_declaration->declaration_node.type = parseType(tokens, token_pos);
    //(*token_pos)++;
    // printf("Resolved type\n");
    // At this point we have an identifier for the variable 
    ast_declaration->declaration_node.identifier = tokens[*token_pos]->tk_value;

    (*token_pos)++;

    return ast_declaration;
}

ASTNode *parseForLoop(Token **tokens, int *token_pos)
{
    //Create for node
    ASTNode *ast_for = __allocateAST();
    ast_for->node_type = NODE_FOR;
    ast_for->for_node.line_number = tokens[*token_pos]->line_number;
    //printf("Creating node for\n");

    //Move past "for" and "("
    (*token_pos)++;
    demand_token(tokens, token_pos,TOKEN_SYMBOL, "(");

    ast_for->for_node.assignment_expr = parseAssignment(tokens, token_pos);
    demand_token(tokens, token_pos,TOKEN_SYMBOL,";");

    ast_for->for_node.condition_expr = parseExpression(tokens, token_pos);
    demand_token(tokens, token_pos,TOKEN_SYMBOL, ";");

    //Check what kind of "stmt" we have now
    StmtKind stmt_kind = classifyStatement(tokens, token_pos);
    if (stmt_kind == STMT_REASSIGNMENT){
        ast_for->for_node.reassignment_expr = parseReassignment(tokens, token_pos);
    }
    else{
        ast_for->for_node.reassignment_expr = parseExpression(tokens, token_pos);
    }

    demand_token(tokens, token_pos,TOKEN_SYMBOL, ")");
    demand_token(tokens, token_pos,TOKEN_SYMBOL, "{");
    //printf("PARSING THE FOR LOOP BLOCK \n");

    ast_for->for_node.body = parseBlock(tokens, token_pos);
    demand_token(tokens, token_pos,TOKEN_SYMBOL, "}");
    return ast_for;
}

ASTNode *parseForeachLoop(Token **tokens, int *token_pos)
{
    //Create foreach node
    ASTNode *ast_foreach = __allocateAST();
    ast_foreach->node_type = NODE_FOREACH;
    ast_foreach->foreach_node.line_number = tokens[*token_pos]->line_number;

    //Move past "foreach" and demand "("
    (*token_pos)++;
    demand_token(tokens,token_pos, TOKEN_SYMBOL,"(");
        
    //Parse the declaration of the iterator
    ast_foreach->foreach_node.foreach_declaration = parseDeclaration(tokens, token_pos);

    //Demand keyword "in"
    demand_token(tokens, token_pos, TOKEN_KEYWORD, "in");
    //Get the array expr
    ast_foreach->foreach_node.iterable_expr = parseExpression(tokens, token_pos);
    
    demand_token(tokens,token_pos, TOKEN_KEYWORD, "within");
    //Get the lower and upper limits
    demand_token(tokens, token_pos, TOKEN_SYMBOL, "(");
    ast_foreach->foreach_node.lower_limit_expr = parseExpression(tokens, token_pos);
    demand_token(tokens, token_pos, TOKEN_SYMBOL, ",");
    ast_foreach->foreach_node.upper_limit_expr = parseExpression(tokens, token_pos);
    demand_token(tokens, token_pos, TOKEN_SYMBOL, ")");
    //Demand closing parenthesis ")"
    demand_token(tokens, token_pos, TOKEN_SYMBOL, ")");
    demand_token(tokens, token_pos, TOKEN_SYMBOL, "{");
    ast_foreach->foreach_node.body = parseBlock(tokens, token_pos);
    demand_token(tokens, token_pos, TOKEN_SYMBOL, "}");


    return ast_foreach;
}

ASTNode *parseFuncCall(Token **tokens, int *token_pos)
{
    // printf("Parsing func call\n");
    //Create a Function Call Node
    ASTNode *ast_funccall = __allocateAST();
    ast_funccall->node_type = NODE_FUNC_CALL;
    ast_funccall->funccall_node.line_number = tokens[*token_pos]->line_number;
    ast_funccall->funccall_node.identifier = strdup(tokens[*token_pos]->tk_value);
    ast_funccall->funccall_node.params_expr = malloc(INIT_FUNCCALL_PARAM_CAPACITY * sizeof(ASTNode *));
    ast_funccall->funccall_node.params_type = malloc(INIT_FUNCCALL_PARAM_CAPACITY * sizeof(Type *));
    ast_funccall->funccall_node.param_capacity = INIT_FUNCCALL_PARAM_CAPACITY;
    ast_funccall->funccall_node.params_count = 0;
    ast_funccall->funccall_node.function = NULL;
    //Move past the identifier and "("
    (*token_pos) += 2;

    while (strcmp(tokens[*token_pos]->tk_value, ")") != 0 && tokens[*token_pos]->tk_type != TOKEN_EOF)
    {
        if (ast_funccall->funccall_node.params_count + 1 >= ast_funccall->funccall_node.param_capacity){
            ast_funccall->funccall_node.param_capacity *= FUNCCALL_PARAM_RESIZE_FACTOR;
            ast_funccall->funccall_node.params_expr = realloc(ast_funccall->funccall_node.params_expr, ast_funccall->funccall_node.param_capacity * sizeof(ASTNode *));
        }
        ast_funccall->funccall_node.params_expr[ast_funccall->funccall_node.params_count] = parseExpression(tokens, token_pos);
        ast_funccall->funccall_node.params_count++;
        expect_token(tokens, token_pos,TOKEN_SYMBOL, ",");
    }

    demand_token(tokens, token_pos,TOKEN_SYMBOL, ")");

    // printf("Finished\n");
    return ast_funccall;
}

ASTNode *parseFuncDef(Token **tokens, int *token_pos)
{
    // printf("Parsing func def\n");
    // Create func node
    ASTNode *ast_func_def = __allocateAST();
    ast_func_def->funcdef_node.params = malloc(INIT_FUNCDEF_PARAM_CAPACITY * sizeof(Parameter *));
    ast_func_def->node_type = NODE_FUNC_DEF;
    ast_func_def->funcdef_node.line_number = tokens[*token_pos]->line_number;
    ast_func_def->funcdef_node.already_built = 0;
    ast_func_def->funcdef_node.params_count = 0;
    ast_func_def->funcdef_node.is_forward = 0;
    ast_func_def->funcdef_node.param_capacity = INIT_FUNCDEF_PARAM_CAPACITY;

    if (strcmp(tokens[*token_pos]->tk_value, "forward") == 0){
        (*token_pos)++;
        ast_func_def->funcdef_node.is_forward = 1;
    }

    // Advance past func
    (*token_pos)++;
    ast_func_def->funcdef_node.return_type = parseType(tokens, token_pos);
    // printf("Parsed a return type sitting at: %p\n", ast_func_def->funcdef_node.return_type);
    ast_func_def->funcdef_node.func_name = tokens[*token_pos]->tk_value;
    // Advance past function name
    (*token_pos)++;
    demand_token(tokens, token_pos, TOKEN_SYMBOL, "(");
    // printf("Passed it \n");
    
    // printf("Current token is: %s \n", tokens[*token_pos]->tk_value);
    // Parameters
    while (strcmp(tokens[*token_pos]->tk_value, ")") != 0)
    {
        if (ast_func_def->funcdef_node.params_count + 1 >= ast_func_def->funcdef_node.param_capacity){
            ast_func_def->funcdef_node.param_capacity *= FUNCDEF_PARAM_RESIZE_FACTOR;
            ast_func_def->funcdef_node.params = realloc(ast_func_def->funcdef_node.params, ast_func_def->funcdef_node.param_capacity * sizeof(Parameter *));
        }
        // printf("Parsing func parameter\n");
        ast_func_def->funcdef_node.params[ast_func_def->funcdef_node.params_count] = parseFunctionParameter(tokens,token_pos);
        ast_func_def->funcdef_node.params_count++;
        expect_token(tokens, token_pos, TOKEN_SYMBOL, ",");
    }

    demand_token(tokens, token_pos, TOKEN_SYMBOL, ")");


    if (ast_func_def->funcdef_node.is_forward == 1){
        demand_token(tokens, token_pos, TOKEN_SYMBOL, ";");
        return ast_func_def;
    }

    demand_token(tokens, token_pos, TOKEN_SYMBOL, "{");
    ast_func_def->funcdef_node.body = parseBlock(tokens, token_pos);
    demand_token(tokens, token_pos, TOKEN_SYMBOL, "}");
    return ast_func_def;
}

ASTNode *parseIf(Token **tokens, int *token_pos)
{
    //Create if node
    ASTNode *ast_if = __allocateAST();
    ast_if->node_type = NODE_IF;
    ast_if->if_node.line_number = tokens[*token_pos]->line_number;
    //Advance past if
    (*token_pos)++;
    demand_token(tokens, token_pos,TOKEN_SYMBOL, "(");
    ast_if->if_node.condition_expr = parseExpression(tokens, token_pos);
    demand_token(tokens, token_pos,TOKEN_SYMBOL, ")");
    demand_token(tokens, token_pos, TOKEN_SYMBOL, "{");
    ast_if->if_node.body = parseBlock(tokens, token_pos);

    demand_token(tokens, token_pos, TOKEN_SYMBOL, "}");

    // Check for elseif bodies 
    ast_if->if_node.elseif_nodes = malloc(sizeof(ASTNode*) * INIT_ELSEIF_CAPACITY);
    ast_if->if_node.elseif_count = 0;
    ast_if->if_node.elseif_capacity = INIT_ELSEIF_CAPACITY;

    while (expect_token(tokens, token_pos, TOKEN_KEYWORD, "elseif"))
    {

        demand_token(tokens, token_pos, TOKEN_SYMBOL, "(");

        ast_if->if_node.elseif_nodes[ast_if->if_node.elseif_count] = __allocateAST();
        ast_if->if_node.elseif_nodes[ast_if->if_node.elseif_count]->node_type = NODE_ELSEIF;
        ast_if->if_node.elseif_nodes[ast_if->if_node.elseif_count]->elseif_node.line_number = tokens[*token_pos]->line_number;

        ast_if->if_node.elseif_nodes[ast_if->if_node.elseif_count]->elseif_node.condition_expr = parseExpression(tokens, token_pos);
        demand_token(tokens, token_pos, TOKEN_SYMBOL, ")");
        demand_token(tokens, token_pos, TOKEN_SYMBOL, "{");
        ast_if->if_node.elseif_nodes[ast_if->if_node.elseif_count]->elseif_node.body = parseBlock(tokens, token_pos);
        demand_token(tokens, token_pos, TOKEN_SYMBOL, "}");

        // Reallocate for more space if needed
        if (ast_if->if_node.elseif_count + 1 >= ast_if->if_node.elseif_capacity){
            ast_if->if_node.elseif_capacity *= IF_ELSEIF_RESIZE_FACTOR;
            ast_if->if_node.elseif_nodes = realloc(ast_if->if_node.elseif_nodes, sizeof(ASTNode *) * ast_if->if_node.elseif_capacity);
        }

        ast_if->if_node.elseif_count++;
    }

    //Check for else body
    ast_if->if_node.else_body = NULL;
    if (expect_token(tokens, token_pos,TOKEN_KEYWORD, "else")){
        demand_token(tokens, token_pos,TOKEN_SYMBOL, "{");
        ast_if->if_node.else_body = parseBlock(tokens, token_pos);
        demand_token(tokens, token_pos,TOKEN_SYMBOL, "}");
    }
    return ast_if;
}

ASTNode *parseObjectDef(Token **tokens, int *token_pos)
{
    // Create object node
    ASTNode *ast_object = __allocateAST();
    ast_object->node_type = NODE_OBJECT;
    ast_object->object_node.line_number = tokens[*token_pos]->line_number;
    ast_object->object_node.is_forward = 0;

    if (strcmp(tokens[*token_pos]->tk_value, "forward") == 0){
        ast_object->object_node.is_forward = 1; 
        (*token_pos)++;
    }

    // Move past the 'object' keyword. Get the object name
    (*token_pos)++;
    ast_object->object_node.identifier = tokens[*token_pos]->tk_value;
    // Create the object type
    Type *object_type = parseType(tokens, token_pos);
    // object_type->udt.udt_kind = UDT_OBJECT;
    // object_type->representation = updateTypeRepresentation(object_type);
    ast_object->object_node.object_type = object_type;
    ast_object->object_node.parent = NULL;  
    // Move past the identifier
    //(*token_pos)++;
       
    // Check whether current object definition inherits another one
    if (expect_token(tokens, token_pos, TOKEN_KEYWORD, "inherits")){
        ast_object->object_node.parent = tokens[*token_pos]->tk_value;
        (*token_pos)++;
    }

    if (ast_object->object_node.is_forward == 1){
        demand_token(tokens, token_pos, TOKEN_SYMBOL, ";");
        return ast_object;
    }

    demand_token(tokens, token_pos, TOKEN_SYMBOL, "{");

    // Allocate for block of declarations
    ast_object->object_node.declaration_count = 0;
    ast_object->object_node.declaration_capacity = INIT_OBJECT_DECL_CAPACITY;
    ast_object->object_node.declarations = malloc(INIT_OBJECT_DECL_CAPACITY * sizeof(ASTNode *));


    while (strcmp(tokens[*token_pos]->tk_value, "}") != 0)
    {
        if (ast_object->object_node.declaration_count + 1 >= ast_object->object_node.declaration_capacity){
            ast_object->object_node.declaration_capacity *= OBJECT_DECL_RESIZE_FACTOR;
            ast_object->object_node.declarations = realloc(ast_object->object_node.declarations, ast_object->object_node.declaration_capacity * sizeof(ASTNode *));
        }

        StmtKind stmt_kind = classifyStatement(tokens, token_pos);
        switch(stmt_kind){
            case STMT_DECLARATION:{
                ast_object->object_node.declarations[ast_object->object_node.declaration_count] = parseDeclaration(tokens, token_pos);
                break;
            }
            case STMT_UNION:{
                ast_object->object_node.declarations[ast_object->object_node.declaration_count] = parseUnion(tokens, token_pos);
                break;
            }
            case STMT_EMPTY:{
                expect_token(tokens, token_pos, TOKEN_SYMBOL, ";");
                break;
            }
            default:{
                printf("In file: '%s'. In L = %i. Unexpected statement in object def, only declarations and unions are allowed, but got: '%s'.\n", 
                    tracker.current_src_file, tracker.current_line, statementToStr(stmt_kind));
                exit(1);
                break;
            }
        }
        ast_object->object_node.declaration_count++;
        demand_token(tokens, token_pos, TOKEN_SYMBOL, ";");
    }
    demand_token(tokens, token_pos, TOKEN_SYMBOL, "}");
    return ast_object;
}

ASTNode *parseUnion(Token **tokens, int *token_pos)
{
    ASTNode *node = __allocateAST();
    node->node_type = NODE_UNION;
    node->union_node.line_number = tokens[*token_pos]->line_number;
    node->union_node.declaration_capacity = INIT_UNION_DECL_CAPACITY;
    node->union_node.declaration_count = 0;
    node->union_node.declarations = malloc(sizeof(ASTNode *) * INIT_UNION_DECL_CAPACITY);

    (*token_pos) += 2; // advance past union {

    while (strcmp(tokens[*token_pos]->tk_value, "}") != 0){
        if (node->union_node.declaration_count + 1 >= node->union_node.declaration_capacity){
            node->union_node.declaration_capacity *= UNION_DECL_RESIZE_FACTOR;
            node->union_node.declarations = realloc(node->union_node.declarations, node->union_node.declaration_capacity * sizeof(ASTNode *));
        }

        node->union_node.declarations[node->union_node.declaration_count++] = parseDeclaration(tokens, token_pos);
        demand_token(tokens, token_pos, TOKEN_SYMBOL, ";");

    }

    demand_token(tokens, token_pos, TOKEN_SYMBOL, "}");
    return node;
}

ASTNode *parseReassignment(Token **tokens, int *token_pos)
{
    ASTNode *node = __allocateAST();
    node->node_type = NODE_REASSIGNMENT;
    node->reassignment_node.line_number = tokens[*token_pos]->line_number;
    //printf("About to parse LValue\n");
    node->reassignment_node.lvalue = parseLValue(tokens, token_pos);



    //printf("About to parse RValue. Current token = %s \n", tokens[*token_pos]->tk_value);
    node->reassignment_node.line_number = tokens[*token_pos]->line_number;
    node->reassignment_node.op = tokens[*token_pos]->tk_type;
    (*token_pos)++;
    node->reassignment_node.expression = parseExpression(tokens, token_pos);

    return node;
}

ASTNode *parseReturn(Token **tokens, int *token_pos)
{
    // Create return node
    ASTNode *ast_return = malloc(sizeof(ASTNode));
    ast_return->node_type = NODE_RETURN;
    ast_return->return_node.line_number = tokens[*token_pos]->line_number;

    // Advance past 'return'
    (*token_pos)++;

    // Check if there's an expression or if return is void type
    if (strcmp(tokens[*token_pos]->tk_value, ";") == 0){
        ast_return->return_node.return_expr = NULL;
        return ast_return;
    }

    ast_return->return_node.return_expr = parseExpression(tokens, token_pos);
    return ast_return;
}

ASTNode *parseContinue(Token **tokens, int *token_pos)
{
    ASTNode *ast_continue = __allocateAST();
    ast_continue->node_type = NODE_CONTINUE;
    ast_continue->continue_node.line_number = tokens[*token_pos]->line_number;
    ast_continue->continue_node.tag = NULL;
    (*token_pos)++;
    return ast_continue;   
}

ASTNode *parseBreak(Token **tokens, int *token_pos)
{
    ASTNode *ast_break = __allocateAST();
    ast_break->node_type = NODE_BREAK;
    ast_break->break_node.line_number = tokens[*token_pos]->line_number;
    ast_break->break_node.tag = NULL;
    (*token_pos)++;
    return ast_break;
}



ASTNode *parseUseDirective(Token **tokens, int *token_pos)
{
 // -------- RESOLVE SYSTEM LIB DIR -------- //
    char *system_dir = get_system_lib_dir();
    char *system_lib_dir = path_join(system_dir, "berries");

    // printf("system lib dir = %s\n", system_lib_dir);

    ASTNode *ast_node = malloc(sizeof(ASTNode));
    ast_node->node_type = NODE_USE_DIRECTIVE;
    ast_node->use_node.line_number = tokens[*token_pos]->line_number;

    // Advance past # and "use"
    (*token_pos) += 2;
    char *requested_path = tokens[*token_pos]->tk_value;

    // Candidate 1: relative to directory of current file
    char *including_dir = get_directory_path(tracker.current_src_file);
    char *candidate1 = path_join(including_dir, requested_path);
    free(including_dir);

    // Candidate 2: relative to cwd
    char cwd[512];
    char *candidate2 = NULL;
    if (getcwd(cwd, sizeof(cwd)) != NULL) {
        candidate2 = path_join(cwd, requested_path);
    } else {
        candidate2 = strdup(requested_path);
    }

    // Candidate 3: system lib dir
    char *candidate3 = path_join(system_lib_dir, requested_path);

    // Choose the first existing one
    char *resolved_path = NULL;
    if (file_exists(candidate1)) {
        resolved_path = strdup(candidate1);
        free(candidate2);
        free(candidate3);
    }
    else if (file_exists(candidate2)) {
        resolved_path = strdup(candidate2);
        free(candidate1);
        free(candidate3);
    }
    else if (file_exists(candidate3)) {
        resolved_path = strdup(candidate3);
        free(candidate1);
        free(candidate2);
    }
    else {
        Error *e = invoke(FILE_NOT_FOUND);
        appendMessageToError(e, fetchErrorMsgBasedOnErrorID(e->error_id), requested_path);
        appendMessageToError(e, "Tried:\n  1. '%s'\n  2. '%s'\n  3. '%s'\n", candidate1, candidate2, candidate3);
        appendMessageToError(e, "Hint: set BJORN_LIB_PATH to your bjorn standard library directory.\n");
        appendMessageToError(e, "      e.g. export BJORN_LIB_PATH=/path/to/bjorn-lib/berries\n");
        appendMessageToError(e, "      Add this to your ~/.bashrc or ~/.profile to make it permanent.\n");
        free(candidate1);
        free(candidate2);
        free(candidate3);
        addError(e);
        (*token_pos)++;
        return ast_node;
    }

    // Check for duplicate include
    if (fileIsAlreadyUsed(resolved_path)) {
        free(resolved_path);
        ast_node->node_type = NODE_NULL;
        (*token_pos)++;
        return ast_node;
    }

    addUsedFile(resolved_path);
    ast_node->use_node.filepath = resolved_path;

    // Advance past the string token
    (*token_pos)++;

    // Save context
    char *old_file = tracker.current_src_file;
    int old_line = tracker.current_line;

    // Parse the included file
    tracker.current_src_file = resolved_path;
    char *src = read_file(resolved_path);
    // printf("Tokenizing: '%s'\n", resolved_path);
    Token **file_tokens = tokenize(src, resolved_path);
    int file_tokens_pos = 0;
    // printf("Parsing : '%s'\n", resolved_path);
    ast_node->use_node.program = parseProgram(file_tokens, &file_tokens_pos);

    // printf("Finished parsing: '%s'\n", resolved_path);
    // Restore context
    tracker.current_src_file = old_file;
    tracker.current_line = old_line;
    // printf("Back to parsing: '%s'\n", tracker.current_src_file);
    return ast_node;
}

ASTNode *parseWhileLoop(Token **tokens, int *token_pos)
{
    //Create while node
    ASTNode *ast_while = __allocateAST();
    ast_while->node_type = NODE_WHILE;
    ast_while->while_node.line_number = tokens[*token_pos]->line_number;
    //Move past while keyword and opening parenthesis
    (*token_pos)++;
    demand_token(tokens, token_pos, TOKEN_SYMBOL, "(");
    //Parse condition expression and move past closing parenthesis and opening bracket
    ast_while->while_node.condition_expr = parseExpression(tokens, token_pos);
    demand_token(tokens, token_pos,TOKEN_SYMBOL,")");

    demand_token(tokens, token_pos,TOKEN_SYMBOL, "{");
    //Parse the body and move past closing bracket
    ast_while->while_node.body = parseBlock(tokens, token_pos);

    demand_token(tokens, token_pos,TOKEN_SYMBOL, "}");
    return ast_while;
}

ASTNode *parseStatement(Token **tokens, int *token_pos)
{
    ASTNode *ast_stmt = NULL;
    tracker.current_line = tokens[*token_pos]->line_number; 
    // printf("Parsing stmt at L = %i, current token = %s\n", tracker.current_line, tokens[*token_pos]->tk_value);
    StmtKind stmt_kind = classifyStatement(tokens, token_pos);


    switch (stmt_kind)
    {
        case STMT_ASSIGNMENT:{
            ast_stmt = parseAssignment(tokens, token_pos);
            demand_token(tokens, token_pos, TOKEN_SYMBOL, ";");
            break;
        }
        case STMT_CLASS_DEF:{
            ast_stmt = parseClassDef(tokens, token_pos);
            break;
        }
        case STMT_DECLARATION:{
            ast_stmt = parseDeclaration(tokens, token_pos);
            demand_token(tokens, token_pos, TOKEN_SYMBOL, ";");
            break;
        }
        case STMT_EMPTY:{
            ast_stmt = parseEmpty(tokens, token_pos);
            break;
        }
        case STMT_ENUM_DEF:{
            ast_stmt = parseEnumDef(tokens, token_pos);
            break;
        }
        case STMT_EXPRESSION:{
            ast_stmt = parseExpression(tokens, token_pos);
            demand_token(tokens, token_pos, TOKEN_SYMBOL, ";");
            break;
        }
        case STMT_EXT_FUNC_DEF:{
            ast_stmt = parseExternFuncDef(tokens, token_pos);
            demand_token(tokens, token_pos, TOKEN_SYMBOL, ";");
            break;
        }
        case STMT_EXT_ID_DEF:{
            ast_stmt = parseExternId(tokens, token_pos);
            demand_token(tokens, token_pos, TOKEN_SYMBOL, ";");
            break;
        }
        case STMT_FOR_LOOP:{
            ast_stmt = parseForLoop(tokens, token_pos);
            break;
        }
        case STMT_FOREACH_LOOP:{
            ast_stmt = parseForeachLoop(tokens, token_pos);
            break;
        }
        case STMT_FUNC_CALL:{
            ast_stmt = parseFuncCall(tokens, token_pos);
            demand_token(tokens, token_pos, TOKEN_SYMBOL, ";");
            break;
        }
        case STMT_FUNC_DEF:{
            ast_stmt = parseFuncDef(tokens, token_pos);
            break;
        }
        case STMT_IF:{
            ast_stmt = parseIf(tokens, token_pos);
            break;
        }
        case STMT_OBJECT_DEF:{
            ast_stmt = parseObjectDef(tokens, token_pos);
            break;
        }

        case STMT_UNION:{
            ast_stmt = parseUnion(tokens, token_pos);
            demand_token(tokens, token_pos, TOKEN_SYMBOL, ";");
            break;
        }
        case STMT_REASSIGNMENT:{
            ast_stmt = parseReassignment(tokens, token_pos);
            demand_token(tokens, token_pos, TOKEN_SYMBOL, ";");
            break;
        }
        case STMT_RETURN:{
            ast_stmt = parseReturn(tokens, token_pos);
            demand_token(tokens, token_pos, TOKEN_SYMBOL, ";");
            break;
        }
        case STMT_CONTINUE:{
            ast_stmt = parseContinue(tokens, token_pos);
            demand_token(tokens, token_pos, TOKEN_SYMBOL, ";");
            break;
        }
        case STMT_BREAK:{
            ast_stmt = parseBreak(tokens, token_pos);
            demand_token(tokens, token_pos, TOKEN_SYMBOL, ";");
            break;
        }
        case STMT_USE_DIRECTIVE:{
            ast_stmt = parseUseDirective(tokens, token_pos);
            break;
        }
        case STMT_WHILE_LOOP:{
            ast_stmt = parseWhileLoop(tokens, token_pos);
            break;
        }

        default:{
            fprintf(stderr, "Unknown STMT_KIND to parse\n");
            exit(1);
        }
    }
    ast_stmt->is_stmt = 1;
    // printf("Finished parsing that stmt\n");
    return ast_stmt;
}

ASTNode *parseLValue(Token **tokens, int *token_pos)
{
    return parseLValue_Precedence2(tokens, token_pos);
}

ASTNode *parseArrayInit(Token **tokens, int *token_pos, char *arr_name, Type *declared_type)
{
    ASTNode *ast_array = __allocateAST();
    ast_array->node_type = NODE_ARRAY_INIT;
    ast_array->array_init_node.line_number = tokens[*token_pos]->line_number;
    ast_array->array_init_node.capacity = INIT_ARRINIT_CAPACITY;
    ast_array->array_init_node.elements = malloc(sizeof(ASTNode *) * ast_array->array_init_node.capacity);
    ast_array->array_init_node.arr_name = arr_name;
    ast_array->array_init_node.element_count = 0;
    ast_array->array_init_node.type = declared_type;

    while (tokens[*token_pos]->tk_type != TOKEN_EOF && 
           !(tokens[*token_pos]->tk_type == TOKEN_SYMBOL && strcmp(tokens[*token_pos]->tk_value, "]") == 0))
    {
        if (ast_array->array_init_node.element_count + 1 > ast_array->array_init_node.capacity){
            ast_array->array_init_node.capacity *= ARRINIT_RESIZE_FACTOR;
            ast_array->array_init_node.elements = realloc(ast_array->array_init_node.elements, ast_array->array_init_node.capacity * sizeof(ASTNode *));
        }

        ast_array->array_init_node.elements[ast_array->array_init_node.element_count++] = parseExpression(tokens, token_pos);
        expect_token(tokens, token_pos, TOKEN_SYMBOL, ",");

    }
    
    return ast_array;

}

//Expression handles '||'. Precedence 12.
ASTNode *parseExpression(Token **tokens, int *token_pos)
{
    //Empty expression
    if (strcmp(tokens[*token_pos]->tk_value, ";") == 0)
    {
        ASTNode *ast_empty = parseEmpty(tokens, token_pos);
        return ast_empty;
    }

    // // Reassigments?
    // if (is_reassignment(tokens, *token_pos))
    // {
    //     //printf("It is a reassignment\n");
    //     ASTNode *ast_reassignment = parseReassignment(tokens, token_pos);
    //     ast_reassignment->node_type = NODE_REASSIGNMENT;
    //     return ast_reassignment;
    // }


    ASTNode *left = parseExpr_Precedence11(tokens, token_pos);

    while (tokens[*token_pos]->tk_type == TOKEN_OPERATOR &&
           (strcmp(tokens[*token_pos]->tk_value, "||") == 0))
    {

        ASTNode *ast_bin_op = __allocateAST();
        ast_bin_op->node_type = NODE_BINARY_OP;
        ast_bin_op->binary_op_node.line_number = tokens[*token_pos]->line_number;
        ast_bin_op->binary_op_node.op = tokens[*token_pos]->tk_value;
        //Advance to the next token after operator
        (*token_pos)++;

        //Create the right and binary op node
        ASTNode *right = parseExpr_Precedence11(tokens, token_pos);

        ast_bin_op->binary_op_node.right = right;
        ast_bin_op->binary_op_node.left = left;

            //Maybe there's only a left
        left = ast_bin_op;
    }

    return left;
}

//Expression handles '&&'. Precedence 11.
ASTNode *parseExpr_Precedence11(Token **tokens, int *token_pos)
{
    ASTNode *left = parseExpr_Precedence10(tokens, token_pos);

    while (tokens[*token_pos]->tk_type == TOKEN_OPERATOR &&
        (strcmp(tokens[*token_pos]->tk_value, "&&") == 0))
    {

        ASTNode *ast_bin_op = __allocateAST();
        ast_bin_op->node_type = NODE_BINARY_OP;
        ast_bin_op->binary_op_node.line_number = tokens[*token_pos]->line_number;
        ast_bin_op->binary_op_node.op = tokens[*token_pos]->tk_value;
        //Advance to the next token after operator
        (*token_pos)++;

        //Create the right and binary op node
        ASTNode *right = parseExpr_Precedence10(tokens, token_pos);

        ast_bin_op->binary_op_node.right = right;
        ast_bin_op->binary_op_node.left = left;
        
        ASTNode *number = tryFoldBinary(left, right, ast_bin_op->binary_op_node.op);
        if (number != NULL)
        {
            left = number; 
            continue;
        }

        //Maybe there's only a left
        left = ast_bin_op;
    }

    return left;
}

//Expr handles BITWISE OR "|". Precedence 10
ASTNode *parseExpr_Precedence10(Token **tokens, int *token_pos)
{
    //Left node comes from parsing the term
    ASTNode *left = parseExpr_Precedence9(tokens, token_pos);

    while ( tokens[*token_pos]->tk_type == TOKEN_OPERATOR &&
            ( (strcmp(tokens[*token_pos]->tk_value, "|") == 0)))

    {


        ASTNode *ast_bin_op = __allocateAST();
        ast_bin_op->node_type = NODE_BINARY_OP;
        ast_bin_op->binary_op_node.line_number = tokens[*token_pos]->line_number;
        ast_bin_op->binary_op_node.op = tokens[*token_pos]->tk_value;
        //Advance to the next token after operator
        (*token_pos)++;

        //Create the right and binary op node
        ASTNode *right = parseExpr_Precedence9(tokens, token_pos);

        ast_bin_op->binary_op_node.right = right;
        ast_bin_op->binary_op_node.left = left;
        
        ASTNode *number = tryFoldBinary(left, right, ast_bin_op->binary_op_node.op);
        if (number != NULL)
        {
            left = number; 
            continue;
        }

        //Maybe there's only a left
        left = ast_bin_op;
    }

    return left;
}

//Expr handles XOR "^". Precedence 9
ASTNode *parseExpr_Precedence9(Token **tokens, int *token_pos)
{
    //Left node comes from parsing the term
    ASTNode *left = parseExpr_Precedence8(tokens, token_pos);

    while ( tokens[*token_pos]->tk_type == TOKEN_OPERATOR &&
            ( (strcmp(tokens[*token_pos]->tk_value, "^") == 0)))
    {


        ASTNode *ast_bin_op = __allocateAST();
        ast_bin_op->node_type = NODE_BINARY_OP;
        ast_bin_op->binary_op_node.line_number = tokens[*token_pos]->line_number;
        ast_bin_op->binary_op_node.op = tokens[*token_pos]->tk_value;
        //Advance to the next token after operator
        (*token_pos)++;

        //Create the right and binary op node
        ASTNode *right = parseExpr_Precedence8(tokens, token_pos);

        ast_bin_op->binary_op_node.right = right;
        ast_bin_op->binary_op_node.left = left;
        
        ASTNode *number = tryFoldBinary(left, right, ast_bin_op->binary_op_node.op);
        if (number != NULL)
        {
            left = number; 
            continue;
        }

        //Maybe there's only a left
        left = ast_bin_op;
    }

    return left;
}


//Expr handles BITWISE AND "&". Precedence 8
ASTNode *parseExpr_Precedence8(Token **tokens, int *token_pos)
{
    //Left node comes from parsing the term
    ASTNode *left = parseExpr_Precedence7(tokens, token_pos);

    while ( tokens[*token_pos]->tk_type == TOKEN_OPERATOR &&
            ( (strcmp(tokens[*token_pos]->tk_value, "&") == 0)))

    {


        ASTNode *ast_bin_op = __allocateAST();
        ast_bin_op->node_type = NODE_BINARY_OP;
        ast_bin_op->binary_op_node.line_number = tokens[*token_pos]->line_number;
        ast_bin_op->binary_op_node.op = tokens[*token_pos]->tk_value;
        //Advance to the next token after operator
        (*token_pos)++;

        //Create the right and binary op node
        ASTNode *right = parseExpr_Precedence7(tokens, token_pos);

        ast_bin_op->binary_op_node.right = right;
        ast_bin_op->binary_op_node.left = left;
        
        ASTNode *number = tryFoldBinary(left, right, ast_bin_op->binary_op_node.op);
        if (number != NULL)
        {
            left = number; 
            continue;
        }

        //Maybe there's only a left
        left = ast_bin_op;
    }

    return left;
}

//Expr handles "!=", "==". Precedence 7
ASTNode *parseExpr_Precedence7(Token **tokens, int *token_pos)
{
    //Left node comes from parsing the term
    ASTNode *left = parseExpr_Precedence6(tokens, token_pos);

    while ( tokens[*token_pos]->tk_type == TOKEN_OPERATOR &&
            ( (strcmp(tokens[*token_pos]->tk_value, "!=") == 0) || 
            (strcmp(tokens[*token_pos]->tk_value, "==") == 0 )))

    {


        ASTNode *ast_bin_op = __allocateAST();
        ast_bin_op->node_type = NODE_BINARY_OP;
        ast_bin_op->binary_op_node.line_number = tokens[*token_pos]->line_number;
        ast_bin_op->binary_op_node.op = tokens[*token_pos]->tk_value;
        //Advance to the next token after operator
        (*token_pos)++;

        //Create the right and binary op node
        ASTNode *right = parseExpr_Precedence6(tokens, token_pos);

        ast_bin_op->binary_op_node.right = right;
        ast_bin_op->binary_op_node.left = left;
        
        ASTNode *number = tryFoldBinary(left, right, ast_bin_op->binary_op_node.op);
        if (number != NULL)
        {
            left = number; 
            continue;
        }

        //Maybe there's only a left
        left = ast_bin_op;
    }

    return left;
}

//Expression handles '>','<', "<=",">=". Precedence 6.
ASTNode *parseExpr_Precedence6(Token **tokens, int *token_pos)
{
    //Left node comes from parsing the term
    ASTNode *left = parseExpr_Precedence5(tokens, token_pos);

    while ( tokens[*token_pos]->tk_type == TOKEN_OPERATOR &&
            (  (strcmp(tokens[*token_pos]->tk_value, ">") == 0) ||
            (strcmp(tokens[*token_pos]->tk_value, "<") == 0) ||
            (strcmp(tokens[*token_pos]->tk_value,"<=") == 0)  ||
            (strcmp(tokens[*token_pos]->tk_value, ">=") == 0)))

    {


        ASTNode *ast_bin_op = __allocateAST();
        ast_bin_op->node_type = NODE_BINARY_OP;
        ast_bin_op->binary_op_node.line_number = tokens[*token_pos]->line_number;
        ast_bin_op->binary_op_node.op = tokens[*token_pos]->tk_value;
        //Advance to the next token after operator
        (*token_pos)++;

        //Create the right and binary op node
        ASTNode *right = parseExpr_Precedence5(tokens, token_pos);

        ast_bin_op->binary_op_node.right = right;
        ast_bin_op->binary_op_node.left = left;
        
        ASTNode *number = tryFoldBinary(left, right, ast_bin_op->binary_op_node.op);
        if (number != NULL)
        {
            left = number; 
            continue;
        }

        //Maybe there's only a left
        left = ast_bin_op;
    }

    return left;
}

//Expression handles shift left and right '<<' and '>>'. Precedence 5.
ASTNode *parseExpr_Precedence5(Token **tokens, int *token_pos)
{
    //Left node comes from parsing the term
    ASTNode *left = parseExpr_Precedence4(tokens, token_pos);

    while ( tokens[*token_pos]->tk_type == TOKEN_OPERATOR &&
            ( (strcmp(tokens[*token_pos]->tk_value, ">>") == 0)  ||
            (strcmp(tokens[*token_pos]->tk_value, "<<") == 0 )))

    {


        ASTNode *ast_bin_op = __allocateAST();
        ast_bin_op->node_type = NODE_BINARY_OP;
        ast_bin_op->binary_op_node.line_number = tokens[*token_pos]->line_number;
        ast_bin_op->binary_op_node.op = tokens[*token_pos]->tk_value;
        //Advance to the next token after operator
        (*token_pos)++;

        //Create the right and binary op node
        ASTNode *right = parseExpr_Precedence4(tokens, token_pos);

        ast_bin_op->binary_op_node.right = right;
        ast_bin_op->binary_op_node.left = left;
        
        ASTNode *number = tryFoldBinary(left, right, ast_bin_op->binary_op_node.op);
        if (number != NULL)
        {
            left = number; 
            continue;
        }

        //Maybe there's only a left
        left = ast_bin_op;
    }

    return left;
}

//Expression handles '+' and '-'. Precedence 2.
ASTNode *parseExpr_Precedence4(Token **tokens, int *token_pos)
{
    //Left node comes from parsing the term
    ASTNode *left = parseExpr_Precedence3(tokens, token_pos);

    while (tokens[*token_pos]->tk_type == TOKEN_OPERATOR &&
           (strcmp(tokens[*token_pos]->tk_value, "+") == 0 ||
            strcmp(tokens[*token_pos]->tk_value, "-") == 0))
    {


        ASTNode *ast_bin_op = __allocateAST();
        ast_bin_op->node_type = NODE_BINARY_OP;
        ast_bin_op->binary_op_node.line_number = tokens[*token_pos]->line_number;
        ast_bin_op->binary_op_node.op = tokens[*token_pos]->tk_value;
        //Advance to the next token after operator
        (*token_pos)++;

        //Create the right and binary op node
        ASTNode *right = parseExpr_Precedence3(tokens, token_pos);

        ast_bin_op->binary_op_node.right = right;
        ast_bin_op->binary_op_node.left = left;
        
        ASTNode *number = tryFoldBinary(left, right, ast_bin_op->binary_op_node.op);
        if (number != NULL)
        {
            left = number; 
            continue;
        }       

        //Maybe there's only a left
        left = ast_bin_op;
    }

    return left;
}

//Expression handles "*" and "/". Precedence 3.
ASTNode *parseExpr_Precedence3(Token **tokens, int *token_pos)
{
    //Left node comes from parsing the term
    ASTNode *left = parseExpr_Precedence2(tokens, token_pos);

    while (tokens[*token_pos]->tk_type == TOKEN_OPERATOR &&
           (strcmp(tokens[*token_pos]->tk_value, "*") == 0 ||
            strcmp(tokens[*token_pos]->tk_value, "/") == 0 ||
            strcmp(tokens[*token_pos]->tk_value, "%") == 0))
    {


        ASTNode *ast_bin_op = __allocateAST();
        ast_bin_op->node_type = NODE_BINARY_OP;
        ast_bin_op->binary_op_node.line_number = tokens[*token_pos]->line_number;
        ast_bin_op->binary_op_node.op = tokens[*token_pos]->tk_value;
        //Advance to the next token after operator
        (*token_pos)++;

        //Create the right and binary op node
        ASTNode *right = parseExpr_Precedence2(tokens, token_pos);

        ast_bin_op->binary_op_node.right = right;
        ast_bin_op->binary_op_node.left = left;

        ASTNode *number = tryFoldBinary(left, right, ast_bin_op->binary_op_node.op);
        if (number != NULL)
        {
            left = number; 
            continue;
        }
        //Maybe there's only a left
        left = ast_bin_op;

    }

    return left;
}

// Unary opeartions, only * allowed as lvalue
ASTNode *parseLValue_Precedence2(Token **tokens, int *token_pos)
{
    //printf("PARSING LVALUE PRECEDENCE 2\n");
    ASTNode *operand = NULL;

    while (tokens[*token_pos]->tk_type == TOKEN_OPERATOR &&
           (strcmp(tokens[*token_pos]->tk_value, "*") == 0))
    {
         //Create a Unary Node
        ASTNode *ast_unary = __allocateAST();
        ast_unary->node_type = NODE_UNARY_OP;
        ast_unary->unary_op_node.line_number = tokens[*token_pos]->line_number;
        ast_unary->unary_op_node.op = tokens[*token_pos]->tk_value;
        (*token_pos)++;
        ast_unary->unary_op_node.right = parseExpr_Precedence1(tokens, token_pos);
        

        operand = ast_unary;
    }

    if (operand != NULL) { return operand; }

    return parseExpr_Precedence1(tokens, token_pos);

}

// Unary operations (unary -, address &, dereference *, logic not !, negate ~ ). Precedence 2
ASTNode *parseExpr_Precedence2(Token **tokens, int *token_pos)
{


    ASTNode *right = NULL;

    if (tokens[*token_pos]->tk_type == TOKEN_OPERATOR &&
           (strcmp(tokens[*token_pos]->tk_value, "*") == 0 ||
            strcmp(tokens[*token_pos]->tk_value, "&") == 0 ||
            strcmp(tokens[*token_pos]->tk_value, "-") == 0 || 
            strcmp(tokens[*token_pos]->tk_value, "!") == 0 || 
            strcmp(tokens[*token_pos]->tk_value, "~") == 0))
    {
         //Create a Unary Node
        ASTNode *ast_unary = __allocateAST();
        ast_unary->node_type = NODE_UNARY_OP;
        ast_unary->unary_op_node.line_number = tokens[*token_pos]->line_number;
        ast_unary->unary_op_node.op = tokens[*token_pos]->tk_value;
        (*token_pos)++;

        ast_unary->unary_op_node.right = parseExpr_Precedence1(tokens, token_pos);

        ASTNode *ast_number = tryFoldUnary(ast_unary->unary_op_node.right, ast_unary->unary_op_node.op);

        if (ast_number != NULL)
        {
            right = ast_number;
            return right;
        }

        right = ast_unary;
    }

    if (right != NULL) { return right; }

    return parseExpr_Precedence1(tokens, token_pos);
}

// Subscription and field access. [] and . and -> 
// Precedence 1.
ASTNode *parseExpr_Precedence1(Token **tokens, int *token_pos)
{

    ASTNode *base = parseUnit(tokens, token_pos);



    while ( (tokens[*token_pos]->tk_type == TOKEN_SYMBOL &&
           (strcmp(tokens[*token_pos]->tk_value, "[") == 0 ||
            strcmp(tokens[*token_pos]->tk_value, ".") == 0 || 
            strcmp(tokens[*token_pos]->tk_value, "->") == 0)) || 

            (tokens[*token_pos]->tk_type == TOKEN_OPERATOR && 
            (strcmp(tokens[*token_pos]->tk_value,"++") == 0 || 
            strcmp(tokens[*token_pos]->tk_value, "--") == 0)))
    {
        // Subscription 
        if (strcmp(tokens[*token_pos]->tk_value, "[") == 0)
        {
            (*token_pos)++;
            ASTNode *ast_subscript = __allocateAST();
            ast_subscript->node_type = NODE_SUBSCRIPT;
            ast_subscript->subscript_node.line_number = tokens[*token_pos]->line_number;
            ast_subscript->subscript_node.line_number = tokens[*token_pos]->line_number;
            ast_subscript->subscript_node.base = base;

            ast_subscript->subscript_node.index = parseExpression(tokens, token_pos);
            demand_token(tokens, token_pos, TOKEN_SYMBOL, "]");
            base = ast_subscript;
        }
        
        // Field access OR method dispatch
        else if (strcmp(tokens[*token_pos]->tk_value, ".") == 0)
        {
            (*token_pos)++;
            // Method dispatch
            if (isFunctionCall(tokens, token_pos))
            {
                ASTNode *ast_method_dispatch = __allocateAST();
                ast_method_dispatch->node_type = NODE_METHOD_DISPATCH;
                ast_method_dispatch->method_dispatch.line_number = tokens[*token_pos]->line_number;
                ast_method_dispatch->method_dispatch.base = base;
                ast_method_dispatch->method_dispatch.func_call = parseFuncCall(tokens, token_pos);
                base = ast_method_dispatch;
            }
            // Field access
            else 
            {
                ASTNode *ast_field_access = __allocateAST();
                ast_field_access->node_type = NODE_FIELD_ACCESS;
                ast_field_access->field_access_node.line_number = tokens[*token_pos]->line_number;
                ast_field_access->field_access_node.base = base; 
                ast_field_access->field_access_node.field_name = tokens[*token_pos]->tk_value;
                (*token_pos)++;
                base = ast_field_access;
            }


        }

        // Ptr field access OR ptr method dispatch
        else if (strcmp(tokens[*token_pos]->tk_value, "->") == 0)
        {
            (*token_pos)++;

            // Ptr Method dispatch
            if (isFunctionCall(tokens, token_pos))
            {
                ASTNode *ast_method_dispatch = __allocateAST();
                ast_method_dispatch->node_type = NODE_PTR_METHOD_DISPATCH;
                ast_method_dispatch->method_dispatch.line_number = tokens[*token_pos]->line_number;
                ast_method_dispatch->method_dispatch.base = base;
                ast_method_dispatch->method_dispatch.func_call = parseFuncCall(tokens, token_pos);
                base = ast_method_dispatch;
            }
            else 
            {
                ASTNode *ast_ptr_field_access = __allocateAST();
                ast_ptr_field_access->node_type = NODE_PTR_FIELD_ACCESS;
                ast_ptr_field_access->ptr_field_access_node.line_number = tokens[*token_pos]->line_number;
                ast_ptr_field_access->ptr_field_access_node.base = base;
                ast_ptr_field_access->ptr_field_access_node.field_name = tokens[*token_pos]->tk_value;
                (*token_pos)++;

                base = ast_ptr_field_access;
            }

        }

        // Post fix operators
        else if (strcmp(tokens[*token_pos]->tk_value, "++") == 0 || strcmp(tokens[*token_pos]->tk_value, "--") == 0)
        {
            ASTNode *ast_postfix_op = __allocateAST();
            ast_postfix_op->node_type = NODE_POSTFIX_OP;
            ast_postfix_op->postfix_op_node.line_number = tokens[*token_pos]->line_number;
            ast_postfix_op->postfix_op_node.left = base;
            ast_postfix_op->postfix_op_node.op = tokens[*token_pos]->tk_value;
            (*token_pos)++;

            base = ast_postfix_op;
        }

    }

    return base;
}

ASTNode *parseChar(Token **tokens, int *token_pos)
{
    //Create a char node
    ASTNode *ast_char = __allocateAST();
    ast_char->node_type = NODE_CHAR;
    ast_char->char_node.line_number = tokens[*token_pos]->line_number;
    ast_char->char_node.char_value = tokens[*token_pos]->tk_value[0];
    ast_char->char_node.char_type = findNumberTypeInTable(1, 1, NUMBER_CHAR);
    (*token_pos)++;

    return ast_char;
}

ASTNode *parseString(Token **tokens, int *token_pos)
{

    //Create a str node
    ASTNode *ast_str = __allocateAST();
    ast_str->node_type = NODE_STR;
    ast_str->str_node.line_number = tokens[*token_pos]->line_number;
    ast_str->str_node.str_value = tokens[*token_pos]->tk_value;
    ast_str->str_node.str_type = findBuiltinInTable("ptr<char>");
    (*token_pos)++;
    return ast_str;
}

ASTNode *parseNumber(Token **tokens, int *token_pos)
{
    //Create a Number node
    ASTNode *ast_number = __allocateAST();
    ast_number->node_type = NODE_NUMBER;
    ast_number->number_node.line_number = tokens[*token_pos]->line_number;
    const char *str = tokens[*token_pos]->tk_value;
    char *endptr;
    uint64_t n = strToNumber(str, &endptr);
    ast_number->number_node.number_type = resolveNumberType(n, 1);
    ast_number->number_node.number_value = n;
    //Advance post the number
    (*token_pos)++;

    return ast_number;
}

ASTNode *parseBoolean(Token **tokens, int *token_pos)
{
    //Create a Bool node
    ASTNode *ast_bool = __allocateAST();
    ast_bool->node_type = NODE_BOOL;
    ast_bool->bool_node.line_number = tokens[*token_pos]->line_number;
    ast_bool->bool_node.bool_value = tokens[*token_pos]->tk_value;
    ast_bool->bool_node.bool_type = findNumberTypeInTable(1, 1, NUMBER_BOOL);
    //Advance post bool value
    (*token_pos)++;

    return ast_bool;
}

ASTNode *parseIdentifier(Token **tokens, int *token_pos)
{
    //Create an Identifier Node
    ASTNode *ast_id = __allocateAST();
    ast_id->node_type = NODE_IDENTIFIER;
    ast_id->identifier_node.line_number = tokens[*token_pos]->line_number;
    ast_id->identifier_node.name = tokens[*token_pos]->tk_value;
    //Advance post the identifier
    (*token_pos)++;

    return ast_id;
}

ASTNode *parseSizeOf(Token **tokens, int *token_pos)
{

    //Advance past "sizeof"
    (*token_pos)++;
    //Create a sizeof node
    ASTNode *ast_sizeof = __allocateAST();
    ast_sizeof->node_type = NODE_SIZEOF;
    ast_sizeof->sizeof_node.line_number = tokens[*token_pos]->line_number;
    demand_token(tokens, token_pos, TOKEN_SYMBOL, "(");

    // Try resolve type
    int token_pos_copy = *token_pos;
    Type *potential_type  = tryParseType(tokens, &token_pos_copy);

    if (potential_type == NULL || !isValidSizeofType(potential_type)){
        // printf("Parsing sizeof expr, curr token = %s\n", tokens[*token_pos]->tk_value);
        ast_sizeof->sizeof_node.sizeof_type = SIZEOF_EXPR;
        ast_sizeof->sizeof_node.expr = parseExpression(tokens, token_pos);
    }
    else{
        // printf("Parsing sizeof type\n");

        ast_sizeof->sizeof_node.sizeof_type = SIZEOF_TYPE;
        ast_sizeof->sizeof_node.type = parseType(tokens, token_pos);
    }
    demand_token(tokens, token_pos, TOKEN_SYMBOL, ")");

    return ast_sizeof;
}

ASTNode *parseCast(Token **tokens, int *token_pos)
{
    ASTNode *ast_cast = __allocateAST();
    ast_cast->node_type = NODE_CAST;
    ast_cast->cast_node.line_number = tokens[*token_pos]->line_number;
    //Get past "("
    (*token_pos)++;

    //Else type is immediate
    ast_cast->cast_node.castType = parseType(tokens, token_pos);

    demand_token(tokens, token_pos, TOKEN_SYMBOL, ")");
    ast_cast->cast_node.expr = parseExpression(tokens, token_pos);
    return ast_cast;
}

ASTNode *parseParenthesis(Token **tokens, int *token_pos)
{
    //Move past the parenthesis
    (*token_pos) ++;

    ASTNode *expr = parseExpression(tokens, token_pos);
    //Remember to get past the closing one

    demand_token(tokens, token_pos,TOKEN_SYMBOL, ")");
    return expr;
}

ASTNode *parseUnaryOp(Token **tokens, int *token_pos)
{
    ASTNode *ast_unary = __allocateAST();
    ast_unary->node_type = NODE_UNARY_OP;
    ast_unary->unary_op_node.line_number = tokens[*token_pos]->line_number;
    ast_unary->unary_op_node.op = tokens[*token_pos]->tk_value;
    (*token_pos)++;

    ast_unary->unary_op_node.right = parseUnit(tokens, token_pos);

    return ast_unary;
}

ASTNode *parseUnit(Token **tokens, int *token_pos)
{
    ASTNode *ast_unit = NULL;
    UnitKind unit_kind = classifyUnit(tokens, token_pos);
    tracker.current_line = tokens[*token_pos]->line_number;

    switch(unit_kind)
    {
        case UNIT_BOOL:{
            ast_unit = parseBoolean(tokens, token_pos);
            break;
        }
        case UNIT_CAST:{
            ast_unit = parseCast(tokens, token_pos);
            break;
        }
        case UNIT_CHAR:{
            ast_unit = parseChar(tokens, token_pos);
            break;
        }
        case UNIT_FUNC_CALL:{
            ast_unit = parseFuncCall(tokens, token_pos);
            break;
        }
        case UNIT_IDENTIFIER:{
            ast_unit = parseIdentifier(tokens, token_pos);
            break;
        }
        case UNIT_NUMBER:{
            ast_unit = parseNumber(tokens, token_pos);
            break;
        }
        case UNIT_PARENTHESIS:{
            ast_unit = parseParenthesis(tokens, token_pos);
            break;
        }
        case UNIT_SIZEOF:{
            ast_unit = parseSizeOf(tokens, token_pos);
            break;
        }
        case UNIT_STR:{
            ast_unit = parseString(tokens, token_pos);
            break;
        }
        case UNIT_UNARYOP:{
            ast_unit = parseUnaryOp(tokens, token_pos);
            break;
        }
    }

    return ast_unit;
}

/**
 * ===============================================================================
 * END AST PARSING   
 * ===============================================================================   
 */



/**
 * ===============================================================================
 * OPTIMIZATIONS  
 * ===============================================================================   
 */


ASTNode *tryFoldBinary(ASTNode *left, ASTNode *right, char *op)
{
    if (left->node_type != NODE_NUMBER || right->node_type != NODE_NUMBER) { return NULL; }

    uint64_t lnumber = left->number_node.number_value;
    uint64_t rnumber = right->number_node.number_value;

    uint64_t result = foldBinOperation(lnumber, rnumber, op);

    ASTNode *ast_number = malloc(sizeof(ASTNode));
    ast_number->node_type = NODE_NUMBER;
    ast_number->number_node.number_value = result;
    ast_number->number_node.number_type = resolveNumberType(result, 1);

    return ast_number;
}


ASTNode *tryFoldUnary(ASTNode *right, char *op)
{
    if (right->node_type == NODE_UNARY_OP){

        ASTNode *folded_inner = tryFoldUnary(right->unary_op_node.right, right->unary_op_node.op);
        if (folded_inner != NULL){
            ASTNode *result = tryFoldUnary(folded_inner, op);
            return result;
        }
        return NULL;
    }

    if (right->node_type != NODE_NUMBER) { return NULL; }

    uint64_t rnumber = right->number_node.number_value;

    //printf("About to fold unary, rtype = %s\n", typeToString(right->number_node.number_type));
    // Only "foldable" Unary operations are: "-" and "!"
    if (strcmp(op, "-") != 0 && strcmp(op, "!") != 0 && strcmp(op, "~")) { 
        return NULL; 
    }

    ASTNode *ast_number = __allocateAST();
    ast_number->node_type = NODE_NUMBER;
    // Handle "!"
    if (strcmp(op, "!") == 0){
        ast_number->number_node.number_value = !rnumber;
        return ast_number;
    }
    // Handle "-"
    else if (strcmp(op, "-") == 0){
        ast_number->number_node.number_value = -rnumber;
        ast_number->number_node.number_type = resolveNumberType(rnumber, 0);
    }   
    // Handle "~"
    else{
        ast_number->number_node.number_value = ~rnumber;
        return ast_number;
    }

    //printf("Result type: %s\n",  typeToString(ast_number->number_node.number_type) );
    return ast_number;

}

uint64_t foldBinOperation(uint64_t left, uint64_t right, char *binary_op)
{
    if (strcmp(binary_op, "+") == 0) { return left + right; }
    if (strcmp(binary_op, "-") == 0) { return left - right; }
    if (strcmp(binary_op, "*") == 0) { return left * right; }
    if (strcmp(binary_op, "/") == 0) { return left / right; }
    if (strcmp(binary_op, ">") == 0) { return left > right; }
    if (strcmp(binary_op, ">=") == 0) { return left >= right; }
    if (strcmp(binary_op, "<") == 0) { return left < right; }
    if (strcmp(binary_op, "<=") == 0) { return left <= right; }
    if (strcmp(binary_op, "==") == 0) { return left == right; }
    if (strcmp(binary_op, "!=") == 0) { return left != right; }
    if (strcmp(binary_op, "&&") == 0) { return left && right; }
    if (strcmp(binary_op, "&") == 0)  { return left & right; }
    if (strcmp(binary_op, "|") == 0)  { return left | right; }
    if (strcmp(binary_op, "^") == 0) { return left ^ right; }
    if (strcmp(binary_op, "||") == 0) { return left || right; }
    if (strcmp(binary_op, "<<") == 0) { return left << right; }
    if (strcmp(binary_op, ">>") == 0) { return left >> right; }



    fprintf(stderr, "Unknown binary operation to fold: %s \n", binary_op);
    exit(1);
}


/**
 * ===============================================================================
 * END OPTIMIZATIONS  
 * ===============================================================================   
 */

char *numberKindToStr(NumberKind kind)
{
    switch(kind)
    {
        case NUMBER_BOOL: return "bool";
        case NUMBER_CHAR: return "char";
        case NUMBER_INTEGER: return "integer";
        default:{
            fprintf(stderr, "Unknown number kind to turn to str: %i\n", kind);
            exit(1);
        }
    }

}




void print_Param(Parameter *p)
{   
    if (p->ptype == P_VARARGS){
        printf("Varargs\n");
        return;
    }
    printf("Name: %s, %s", p->name, p->type->representation);
}

void print_ast(ASTNode *program, int indent)
{
    if (program == NULL) { return; }


    for (int i = 0; i < indent; i++) { printf("  "); }

    switch (program->node_type)
    {
        case NODE_ASSIGNMENT:
        {
            printf("Assignment: %s, Type: %s", program->assignment_node.identifier, program->assignment_node.type->representation);
            print_ast(program->assignment_node.expression, indent + 1);
            break;
        }

        case NODE_REASSIGNMENT:
        {
            printf("Reassignment with '%s' \n", tokenTypeToStr(program->reassignment_node.op));

            for (int i = 0; i < indent; i++) printf("  ");
            printf("LValue: \n");
            print_ast(program->reassignment_node.lvalue, indent + 1);

            for (int i = 0; i < indent; i++) printf("  ");
            printf("RValue: \n");
            print_ast(program->reassignment_node.expression, indent + 1);
            break;
        }
        
        case NODE_SUBSCRIPT:
        {
            printf("Subscript: \n");
            for (int i = 0; i < indent; i++) printf("  ");
            printf("Base Array:\n");
            print_ast(program->subscript_node.base, indent + 1);

            for (int i = 0; i < indent; i++) printf("  ");
            printf("Index: \n");
            print_ast(program->subscript_node.index, indent + 1);
            for (int i = 0; i < indent; i++) printf("  ");
            printf("Finished\n");
            break;
        }
        
        case NODE_ARRAY_INIT:
        {
            printf("Array Init. Identifier = %s\n", program->array_init_node.arr_name);
            for (int k = 0; k < program->array_init_node.element_count; k++)
            {
                for (int i = 0; i < indent; i++) printf("  ");
                printf("Element %i = \n", k);
                print_ast(program->array_init_node.elements[k], indent + 1);
            }
            break;
        }

        case NODE_METHOD_DISPATCH:
        {
            printf("Method dispatch:\n");
            print_ast(program->method_dispatch.base, indent  + 1);

            print_ast(program->method_dispatch.func_call, indent + 1);
            break;

        }

        case NODE_PTR_METHOD_DISPATCH:
        {
            printf("Ptr Method dispatch:\n");
            print_ast(program->method_dispatch.base, indent  + 1);

            print_ast(program->method_dispatch.func_call, indent + 1);
            break;

        }

        case NODE_FIELD_ACCESS:
        {
            printf("Field access: \n");
            print_ast(program->field_access_node.base, indent + 1);

            for (int i = 0; i < indent + 1; i++) printf("  ");
            printf("Field name = %s\n", program->field_access_node.field_name);
            break;
        }

        case NODE_PTR_FIELD_ACCESS:
        {
            printf("Ptr field access: \n");
            print_ast(program->ptr_field_access_node.base, indent + 1);

            for (int i = 0; i < indent + 1; i++) { printf("  "); }
            printf("Field name = %s \n", program->ptr_field_access_node.field_name);
            break;

        }

        case NODE_UNION:{
            printf("Union: \n");
            for (int j = 0; j < program->union_node.declaration_count; j++){
                print_ast(program->union_node.declarations[j], indent + 1);
            }
            break;
        }
        

        case NODE_DECLARATION:
        {
            printf("Declaration: %s Type: %s", program->declaration_node.identifier, program->declaration_node.type->representation);
            break;
        }

        case NODE_UNARY_OP:
        {
            printf("Unary op: %s \n", program->unary_op_node.op);
            print_ast(program->unary_op_node.right, indent + 1);
            break;
        }

        case NODE_POSTFIX_OP:
        {
            printf("Postfix op: %s \n", program->postfix_op_node.op);
            print_ast(program->postfix_op_node.left, indent + 1);
            break;
        }

        case NODE_CAST:
        {
            printf("Cast to type: %s\n", program->cast_node.castType->representation);
            print_ast(program->cast_node.expr, indent + 1);
            break;
        }

        case NODE_SIZEOF:
        {
            if (program->sizeof_node.sizeof_type == SIZEOF_TYPE){
                printf("Sizeof : %s\n", program->sizeof_node.type->representation);
            }
            else{
                printf("Sizeof :\n");
                print_ast(program->sizeof_node.expr, indent + 1);
            }
            
            break;
        }
        case NODE_BINARY_OP:
        {
            printf("Binary Op: %s \n", program->binary_op_node.op);
            print_ast(program->binary_op_node.left, indent + 1);
            print_ast(program->binary_op_node.right, indent + 1);
            break;
        }


        case NODE_NULL:
        {
            printf("Empty statement. \n");
            break;
        }

        case NODE_NUMBER:
        {
            printf("Number = %" PRId64 "\n", program->number_node.number_value);
            break;
        }

        case NODE_CHAR:
        {
            printf("Char = %c \n", program->char_node.char_value);
            break;
        }

        case NODE_STR:
        {
            printf("String = %s \n", program->str_node.str_value);
            break;
        }

        case NODE_BOOL:
        {
            printf("Bool = %s \n", program->bool_node.bool_value);
            break;
        }

        case NODE_IDENTIFIER:
        {
            printf("Identifier = %s \n", program->identifier_node.name);
            break;
        }

        case NODE_WHILE:
        {
            printf("While: \n");
            print_ast(program->while_node.condition_expr, indent + 1);
            print_ast(program->while_node.body, indent + 1);
            break;
        }

        case NODE_IF:
        {
            printf("If: \n");
            for (int i = 0; i < indent; i++) {
                printf("  ");
            }
            printf("Condition: \n");
            print_ast(program->if_node.condition_expr, indent + 1);
            print_ast(program->if_node.body, indent + 1);

            for (int i = 0; i < program->if_node.elseif_count; i++)
            {
                for (int i = 0; i < indent; i++) {
                    printf("  ");
                }
                printf("Elseif: \n");
                for (int i = 0; i < indent; i++) {
                    printf("  ");
                } 
                printf("Condition: \n");
                
                print_ast(program->if_node.elseif_nodes[i]->elseif_node.condition_expr, indent + 1);
                print_ast(program->if_node.elseif_nodes[i]->elseif_node.body, indent + 1);
            }



            if (program->if_node.else_body != NULL)
            {
                for (int i = 0; i < indent; i++) printf("  ");
                printf("Else: \n");
                print_ast(program->if_node.else_body, indent + 1);
            }
            break;
        }

        case NODE_CONTINUE:{
            printf("continue \n");
            break;
        }

        case NODE_BREAK:{
            printf("break \n");
            break;
        }

        case NODE_FOR:
        {
            printf("For: \n");
            for (int i = 0; i < indent; i++) {
                printf("  ");
            } 
            printf("Assignment: \n");
            print_ast(program->for_node.assignment_expr, indent + 1);
            for (int i = 0; i < indent; i++) {
                printf("  ");
            } 
            printf("Condition: \n");
            print_ast(program->for_node.condition_expr, indent + 1);
            for (int i = 0; i < indent; i++) {
                printf("  ");
            } 
            printf("Update: \n");
            print_ast(program->for_node.reassignment_expr, indent + 1);
            print_ast(program->for_node.body, indent + 1);
            break;
        }

        case NODE_FOREACH:
        {
            printf("Foreach: \n");
            print_ast(program->foreach_node.foreach_declaration, indent + 1);
            for (int i = 0; i < indent; i++) {
                printf("  ");
            }
            printf("in: \n");
            print_ast(program->foreach_node.iterable_expr, indent + 1);
            for (int i = 0; i < indent; i++){
                printf("  ");
            }
            printf("within: \n");
            for (int i = 0; i < indent; i++) {
                printf("  ");
            }
            printf("Lower limit: \n");
            print_ast(program->foreach_node.lower_limit_expr, indent + 1);

            for (int i = 0; i < indent; i++) {
                printf("  ");
            }
            printf("Upper limit: \n");
            print_ast(program->foreach_node.upper_limit_expr, indent + 1);
            print_ast(program->foreach_node.body, indent + 1);
            break;
        }

        case NODE_FUNC_DEF:
        {
            printf("Func '%s' -> %s", program->funcdef_node.func_name, program->funcdef_node.return_type->representation);
            for (int j = 0; j < program->funcdef_node.params_count; j++)
            {
                for (int k = 0; k < indent + 1; k++) {
                    printf("  ");
                }
                printf("Param %i: ",j);
                print_Param(program->funcdef_node.params[j]);
            }
            if (!program->funcdef_node.is_forward){
                print_ast(program->funcdef_node.body, indent + 1);
            }
            
            break;
        }

        case NODE_EXTERN_FUNC_DEF:
        {
            printf("Extern Func (rt : %s ) def: %s.\n", program->extern_func_def_node.return_type->representation, program->extern_func_def_node.func_name);
            for (int j = 0; j < program->extern_func_def_node.params_count; j++)
            {
                for (int k = 0; k < indent + 1; k++) {
                    printf("  ");
                }
                printf("Param %i: ",j);
                print_Param(program->extern_func_def_node.params[j]);

            }
            break;        
        }
        case NODE_EXTERN_IDENTIFIER:
        {
            printf("Extern Identifier. Type: %s, Name: %s\n", program->extern_identifier_node.type->representation, program->extern_identifier_node.name);
            break;
        }

        case NODE_FUNC_CALL:
        {
            printf("Call %s. Params: \n", program->funccall_node.identifier);
            for (int j = 0; j < program->funccall_node.params_count; j++)
            {
                print_ast(program->funccall_node.params_expr[j], indent + 1);
            }
            break;
        }

        case NODE_RETURN:
        {
            printf("Return: \n");
            if (program->return_node.return_expr == NULL)
            {
                break;
            }
            print_ast(program->return_node.return_expr, indent + 1);
            break;
        }

        case NODE_USE_DIRECTIVE:
        {
            printf("Use directive: %s \n", program->use_node.filepath);
            print_ast(program->use_node.program, indent - 1);
            break;
        }

        case NODE_OBJECT:
        {
            printf("object %s ", program->object_node.identifier);
            if (program->object_node.parent != NULL){
                printf("inherits %s ", program->object_node.parent);
            }
            printf("\n");
            if (program->object_node.is_forward == 1){
                break;
            }
            for (int j = 0; j < program->object_node.declaration_count; j++){
                print_ast(program->object_node.declarations[j], indent + 1);
            }
            break;
        }

        case NODE_CLASS:
        {
            printf("class %s ", program->class_node.identifier);
            if (program->class_node.parent != NULL){
                printf("extends %s ", program->class_node.parent);
            }
            printf("\n");
            if (program->class_node.is_forward == 1){
                break;
            }
            for (int j = 0; j < program->class_node.declaration_count; j++){
                print_ast(program->class_node.declarations[j], indent + 1);
            }
            for (int k = 0; k < program->class_node.funcdefs_count; k++){
                print_ast(program->class_node.func_defs[k], indent + 1);
            }
            break;

        }

        case NODE_ENUM:
        {
            printf("Enum %s \n", program->enum_node.identifier);
            for (int j = 0; j < program->enum_node.declaration_count; j++)
            {
                print_ast(program->enum_node.declarations[j], indent + 1);
            }
            break;
        }


        case NODE_BLOCK:
        {
            printf("Block: \n");
            for (int i = 0; i < program->block_node.statement_count; i++)
            {
                print_ast(program->block_node.statements[i], indent + 1);
            }
            break;
        }

        default:
        {
            printf("Unknown node type in print_ast(): %i\n", program->node_type);
            break;
        }


    }

}


int is_statement(ASTNode *node)
{
    switch(node->node_type)
    {
        case NODE_FUNC_CALL:{
            if (node->is_stmt == 1) { return 1; }
            break;
        }
        case NODE_METHOD_DISPATCH:{
            if (node->is_stmt == 1) { return 1; }
            break;
        }
        case NODE_PTR_METHOD_DISPATCH:{
            if (node->is_stmt == 1) { return 1; }
            break;
        }
        case NODE_ASSIGNMENT:
        case NODE_DECLARATION:
        case NODE_WHILE:
        case NODE_FOR:
        case NODE_RETURN:
        case NODE_FOREACH:
        case NODE_IF:
        case NODE_ELSEIF:
        case NODE_FUNC_DEF:
        case NODE_CLASS:
        case NODE_CONTINUE:
        case NODE_BREAK:
        case NODE_ENUM:
        case NODE_OBJECT:
        case NODE_REASSIGNMENT: return 1;
        
        default: return 0;
    }

    return 0;
}

int is_pushing_scope(ASTNode *node)
{
    switch (node->node_type) 
    {
        case NODE_FUNC_DEF:
        case NODE_IF:
        case NODE_ELSEIF:
        case NODE_FOR:
        case NODE_FOREACH:
        case NODE_CLASS:
        case NODE_OBJECT:
        case NODE_ENUM:
        case NODE_WHILE: return 1;

        default: return 0;
    }
}

char *astTypeToStr(ASTNode *node)
{
    switch(node->node_type)
    {
        case NODE_BINARY_OP: return "BINARY_OP";
        case NODE_RETURN: return "RETURN";
        case NODE_NUMBER: return "NUMBER";
        case NODE_BLOCK: return "BLOCK";
        case NODE_ASSIGNMENT: return "ASSIGNMENT";
        case NODE_REASSIGNMENT: return "REASSIGNMENT";
        case NODE_EXTERN_FUNC_DEF: return "EXTERN_FUNC_DEF";
        case NODE_EXTERN_IDENTIFIER: return "EXTERN_IDENTIFIER";
        case NODE_DECLARATION: return "DECLARATION";
        case NODE_NULL: return "NULL";
        case NODE_ENUM: return "ENUM";
        case NODE_FOREACH: return "FOREACH";
        case NODE_SIZEOF: return "SIZEOF";
        case NODE_CONTINUE: return "CONTINUE";
        case NODE_FUNC_CALL: return "FUNC_CALL";
        case NODE_FUNC_DEF: return "FUNC_DEF";
        case NODE_BREAK: return "BREAK";
        case NODE_SUBSCRIPT: return "SUBSCRIPT";
        case NODE_USE_DIRECTIVE: return "USE_DIRECTIVE";
        case NODE_UNARY_OP: return "UNARY_OP";
        case NODE_BOOL: return "BOOL";
        case NODE_IDENTIFIER: return "IDENTIFIER";
        case NODE_CAST: return "CAST";
        case NODE_CHAR: return "CHAR";
        case NODE_FOR: return "FOR";
        case NODE_IF: return "IF";
        case NODE_ELSEIF: return "ELSEIF";
        case NODE_OBJECT: return "OBJECT";
        case NODE_CLASS: return "CLASS";
        case NODE_STR: return "STRING";
        case NODE_WHILE: return "WHILE";
        case NODE_ARRAY_INIT: return "ARRAY_INIT";
        case NODE_FIELD_ACCESS: return "FIELD_ACCESS";
        case NODE_PTR_FIELD_ACCESS: return "PTR_FIELD_ACCESS";
        case NODE_METHOD_DISPATCH: return "METHOD_DISPATCH";
        case NODE_PTR_METHOD_DISPATCH: return "PTR_METHOD_DISPATCH";
        case NODE_POSTFIX_OP: return "POSTFIX_OP";

        default: 
        {
            printf("%i \n", node->node_type);
            return "UNKNOWN AST TYPE";
        }
    }
}

char *statementToStr(StmtKind stmt_kind)
{
    switch(stmt_kind){
        case STMT_ASSIGNMENT: return "ASSIGNMENT";
        case STMT_BREAK: return "BREAK";
        case STMT_CLASS_DEF: return "CLASS_DEF";
        case STMT_CONTINUE: return "CONTINUE";
        case STMT_DECLARATION: return "DECLARATION";
        case STMT_EMPTY: return "EMPTY";
        case STMT_ENUM_DEF: return "ENUM_DEF";
        case STMT_EXPRESSION: return "EXPRESSION";
        case STMT_EXT_FUNC_DEF: return "EXT_FUNC_DEF";
        case STMT_EXT_ID_DEF: return "EXT_ID_DEF";
        case STMT_FOR_LOOP: return "FOR_LOOP";
        case STMT_FOREACH_LOOP: return "FOREACH_LOOP";
        case STMT_FUNC_CALL: return "FUNC_CALL";
        case STMT_FUNC_DEF: return "FUNC_DEF";
        case STMT_FW_CLASS_DEF: return "FW_CLASS_DEF";
        case STMT_FW_FUNC_DEF: return "FM_FUNC_DEF";
        case STMT_FW_OBJ_DEF: return "FW_OBJ_DEF";
        case STMT_IF: return "IF";
        case STMT_OBJECT_DEF: return "OBJECT_DEF";
        case STMT_REASSIGNMENT: return "REASSIGNMENT";
        case STMT_RETURN: return "RETURN";
        case STMT_UNION: return "UNION";
        case STMT_USE_DIRECTIVE: return "USE_DIRECTIVE";
        case STMT_WHILE_LOOP: return "WHILE_LOOP";
        default: return "UNKNOWN_STMT";
    }
}

int isLvalue(ASTNode *ast_node)
{
    switch(ast_node->node_type)
    {
        case NODE_IDENTIFIER:
        case NODE_FIELD_ACCESS:
        case NODE_PTR_FIELD_ACCESS:
        case NODE_SUBSCRIPT: return 1;
        case NODE_UNARY_OP:{
            if (strcmp(ast_node->unary_op_node.op, "*") == 0){
                return 1;
            }
            return 0;
        }
        default: return 0;
    }
}