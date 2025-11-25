//
// Created by pablo on 25/02/2025.
//

#include "../include/ast.h"
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <inttypes.h>


UsedFiles *usedFiles = NULL;


char *read_file(const char *filename)
{
    FILE *file = fopen(filename, "r");
    if (!file)
    {
        fprintf(stderr, "Error opening file (%s). Not found \n", filename);
        exit(1);
    }


    int buffer_size = 256;
    char *buffer = malloc(buffer_size);
    if (!buffer) {
        perror("Memory allocation failed");
        fclose(file);
        return NULL;
    }

    int index = 0;
    char ch;

    while ((ch = fgetc(file)) != EOF)
    {
        if (index >= buffer_size - 1)
        { // Resize if needed
            buffer_size *= 2;
            buffer = realloc(buffer, buffer_size);
            if (!buffer) {
                perror("Memory reallocation failed");
                fclose(file);
                return NULL;
            }
        }
        buffer[index] = ch;
        index++;
    }

    buffer[index] = '\0'; // Null-terminate the string
    fclose(file);
    return buffer;
}

void free_usedFiles()
{
    for (int i = 0; i < usedFiles->count; i++)
    {
        free(usedFiles->files[i]);
    }
    free(usedFiles->files);
    free(usedFiles);
}

void init_usedFiles(int initial_capacity)
{
    usedFiles = malloc(sizeof(UsedFiles));    
    usedFiles->files = malloc(initial_capacity * sizeof(char *));
    usedFiles->count = 0;
    usedFiles->capacity = initial_capacity;
}

int is_already_used(char *filename)
{
    for (int i = 0; i < usedFiles->count; i++)
    {
        if (strcmp(filename, usedFiles->files[i]) == 0)
        {
            return 1;
        }
    }
    return 0;
}

int is_assignable_op(Token **tokens, int *token_pos, int offset)
{
    if (tokens[*token_pos + offset]->tk_type == TOKEN_ASSIGN) { return 1; }
    if (tokens[*token_pos + offset]->tk_type == TOKEN_ADD_ASSIGN) { return 1; }
    if (tokens[*token_pos + offset]->tk_type == TOKEN_SUB_ASSIGN) { return 1; }
    if (tokens[*token_pos + offset]->tk_type == TOKEN_MUL_ASSIGN) { return 1; }
    if (tokens[*token_pos + offset]->tk_type == TOKEN_MOD_ASSIGN) { return 1; }
    if (tokens[*token_pos + offset]->tk_type == TOKEN_DIV_ASSIGN) { return 1; }
    return 0;
}

void add_used_file(char *filename)
{
    if (usedFiles->count + 1 > usedFiles->capacity)
    {
        usedFiles->files = realloc(usedFiles->files, 2 * usedFiles->capacity);
        usedFiles->capacity = 2 * usedFiles->capacity;
    }

    usedFiles->files[usedFiles->count] = malloc(strlen(filename) + 1);
    strcpy(usedFiles->files[usedFiles->count], filename);
    usedFiles->count++;
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
        strcmp(value, tokens[*token_pos]->tk_value) == 0)
    {
        (*token_pos)++;
        return;
    }


    fprintf(stderr, "Missing: %s. Instead got: %s. After %s %s %s \n", value, tokens[*token_pos]->tk_value ,tokens[*token_pos - 3]->tk_value,
                            tokens[*token_pos - 2]->tk_value, tokens[*token_pos - 1]->tk_value);
    exit(1);
}

<<<<<<< Updated upstream
=======
int is_type_array(char *type)
{
    int i = 0;
    while (type[i] != '\0')
    {
        if (type[i] == '[')
        {
            return type[i + 1] - '0';
        }
        i++;
    }

    return 0;
}


int is_declaration(Token **tokens, int token_pos)
{
    int token_pos_copy = token_pos;

    if (tokens[token_pos_copy]->tk_type != TOKEN_IDENTIFIER && tokens[token_pos_copy]->tk_type != TOKEN_KEYWORD && 
    tokens[token_pos_copy]->tk_type != TOKEN_TYPE) { return 0; }

    printf("token_pos_copy = %i \n", token_pos_copy);
    char *type = tryResolveType(tokens, &token_pos_copy);
    printf("token_pos_copy = %i \n", token_pos_copy);

    if (tokens[token_pos_copy]->tk_type == TOKEN_IDENTIFIER && strcmp(tokens[token_pos_copy + 1]->tk_value, ";") == 0) { return 1; }

    return 0;
}

int is_assignment(Token **tokens, int token_pos)
{
    printf("Checking if its assignment. Current token = %s  \n", tokens[token_pos]->tk_value);
    int token_pos_copy = token_pos;
    
    if (tokens[token_pos_copy]->tk_type != TOKEN_IDENTIFIER && tokens[token_pos_copy]->tk_type != TOKEN_KEYWORD && 
    tokens[token_pos_copy]->tk_type != TOKEN_TYPE) { printf("here\n"); return 0; }

    printf("token_pos_copy = %i \n", token_pos_copy);
    char *type = tryResolveType(tokens, &token_pos_copy);
    printf("######## tried resolving type: %s \n", type);
    printf("token_pos_copy = %i \n", token_pos_copy);
    if (tokens[token_pos_copy]->tk_type == TOKEN_IDENTIFIER && strcmp(tokens[token_pos_copy + 1]->tk_value, "=") == 0) { return 1; }

    return 0;
}

int is_reassignment(Token **tokens, int token_pos)
{
    printf("Checking if its reassingment. Current token = %s \n", tokens[token_pos]->tk_value);
    int i = token_pos;
    int offset = 0;
    while (tokens[i]->tk_type != TOKEN_EOF)
    {
        if (strcmp(tokens[i]->tk_value, ";") == 0) { return 0; }

        if (is_assignable_op(tokens, &token_pos, offset)) { printf("It is \n"); return 1; }

        i++;
        offset++;
    }

    printf("It's not\n");
    return 0;
}


>>>>>>> Stashed changes
ASTNode *parseProgram(Token **tokens, int *token_pos)
{
    ASTNode *program = malloc(sizeof(ASTNode ));
    program->node_type = NODE_BLOCK;
    program->block_node.statements = malloc(10 * sizeof(ASTNode *));
    program->block_node.statement_count = 0;
    program->block_node.capacity = 10;

    while (tokens[*token_pos]->tk_type != TOKEN_EOF)
    {
        if (program->block_node.statement_count + 1 >= program->block_node.capacity)
        {
            program->block_node.capacity = program->block_node.capacity * 2;
            program->block_node.statements = realloc(program->block_node.statements, program->block_node.capacity * sizeof(ASTNode *));
            if (program->block_node.statements == NULL)
            {
                fprintf(stderr, "Resizing program's block of statements failed.\n");
                exit(1);
            }
        }
        program->block_node.statements[program->block_node.statement_count++] = parseStatement(tokens, token_pos);
    }


    return program;
}


ASTNode *parseBlock(Token **tokens, int *token_pos)
{
    // Create the block node
    ASTNode *block = malloc(sizeof(ASTNode));
    block->node_type = NODE_BLOCK;
    block->block_node.statements = malloc(10 * sizeof(ASTNode *));
    block->block_node.statement_count = 0;
    block->block_node.capacity = 10; // Initial capacity

    // Parse statements until the end of the block or EOF
    while (tokens[*token_pos]->tk_type != TOKEN_EOF && (strcmp(tokens[*token_pos]->tk_value, "}") != 0) )
    {
        // Resize the array if necessary
        if (block->block_node.statement_count + 1>= block->block_node.capacity)
        {
            // Double the capacity (or start with 10 if capacity is 0)
            block->block_node.capacity = block->block_node.capacity * 2;
            block->block_node.statements = realloc(block->block_node.statements, block->block_node.capacity * sizeof(ASTNode *));
            if (!block->block_node.statements)
            {
                fprintf(stderr, "Resizing block of statements failed.\n");
                exit(1);
            }
        }
        // Parse the next statement and add it to the block
        block->block_node.statements[block->block_node.statement_count++] = parseStatement(tokens, token_pos);
    }

    return block;
}


ASTNode *parseStatement(Token **tokens, int *token_pos)
{
    //Use directives
    if (tokens[*token_pos]->tk_type == TOKEN_SYMBOL && strcmp(tokens[*token_pos]->tk_value, "#") == 0)
    {
        ASTNode *use_node = parseUseDirective(tokens, token_pos);
        return use_node;
    }
    //Empty statements with ";"
    if (tokens[*token_pos]->tk_type == TOKEN_SYMBOL &&
            strcmp(tokens[*token_pos]->tk_value, ";") == 0)
    {
        //Advance to next token
        (*token_pos)++;
        //Create null node and return it
        ASTNode *ast_null = malloc(sizeof(ASTNode ));
        ast_null->node_type = NODE_NULL;
        return ast_null;
    }

    //Function calls:  identifier + "(". So we have a function call as a complete statement
    else if (tokens[*token_pos]->tk_type == TOKEN_IDENTIFIER &&
             strcmp(tokens[*token_pos + 1]->tk_value, "(") == 0)
    {
        ASTNode *node = malloc(sizeof(ASTNode));
        node->node_type = NODE_STDALONE_FUNC_CALL;
        node->stdalone_func_call_node.params_expr = malloc(100 * sizeof(ASTNode *));
        node->stdalone_func_call_node.params_count = 0;

        node->stdalone_func_call_node.identifier = tokens[*token_pos]->tk_value;
        (*token_pos)++;
        demand_token(tokens,token_pos, TOKEN_SYMBOL,"(");
        while (strcmp(tokens[*token_pos]->tk_value, ")") != 0)
        {
            node->stdalone_func_call_node.params_expr[node->stdalone_func_call_node.params_count++] = parseExpression(tokens,token_pos);
            expect_token(tokens,token_pos, TOKEN_SYMBOL,",");
        }
        
        demand_token(tokens,token_pos, TOKEN_SYMBOL,")");
        demand_token(tokens, token_pos, TOKEN_SYMBOL, ";");
        return node;
    }
    //While statement
    else if (tokens[*token_pos]->tk_type == TOKEN_KEYWORD &&
             strcmp(tokens[*token_pos]->tk_value, "while") == 0)
    {

        //Create while node
        ASTNode *ast_while = malloc(sizeof(ASTNode ));
        ast_while->node_type = NODE_WHILE;
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

    //Skip statement 
    else if (tokens[*token_pos]->tk_type == TOKEN_KEYWORD && 
            strcmp(tokens[*token_pos]->tk_value, "skip") == 0)
    {
        ASTNode *ast_skip = malloc(sizeof(ASTNode));
        ast_skip->node_type = NODE_SKIP;
        (*token_pos)++;
        demand_token(tokens, token_pos, TOKEN_SYMBOL, ";");
        return ast_skip;
    }
    //Stop statement 
    else if (tokens[*token_pos]->tk_type == TOKEN_KEYWORD && 
            strcmp(tokens[*token_pos]->tk_value, "stop") == 0)
    {
        ASTNode *ast_stop = malloc(sizeof(ASTNode));
        ast_stop->node_type = NODE_STOP;
        (*token_pos)++;
        demand_token(tokens, token_pos, TOKEN_SYMBOL, ";");
        return ast_stop;
    }


    //If statement
    else if (tokens[*token_pos]->tk_type == TOKEN_KEYWORD &&
            strcmp(tokens[*token_pos]->tk_value, "if") == 0)
    {
        //Create if node
        ASTNode *ast_if = malloc(sizeof(ASTNode ));
        ast_if->node_type = NODE_IF;
        //Advance past if
        (*token_pos)++;
        demand_token(tokens, token_pos,TOKEN_SYMBOL, "(");
        ast_if->if_node.condition_expr = parseExpression(tokens, token_pos);
        demand_token(tokens, token_pos,TOKEN_SYMBOL, ")");
        demand_token(tokens, token_pos, TOKEN_SYMBOL, "{");
        ast_if->if_node.body = parseBlock(tokens, token_pos);
        demand_token(tokens, token_pos, TOKEN_SYMBOL, "}");

        ast_if->if_node.else_body = NULL;
        //Check for else body
        if (expect_token(tokens, token_pos,TOKEN_KEYWORD, "else"))
        {
            demand_token(tokens, token_pos,TOKEN_SYMBOL, "{");
            ast_if->if_node.else_body = parseBlock(tokens, token_pos);
            demand_token(tokens, token_pos,TOKEN_SYMBOL, "}");
        }

        return ast_if;
    }
    // For statement
    else if (tokens[*token_pos]->tk_type == TOKEN_KEYWORD &&
             strcmp(tokens[*token_pos]->tk_value, "for") == 0)
    {
        //Create for node
        ASTNode *ast_for = malloc(sizeof(ASTNode ));
        ast_for->node_type = NODE_FOR;
        printf("Creating node for\n");

        //Move past "for" and "("
        (*token_pos)++;
        demand_token(tokens, token_pos,TOKEN_SYMBOL, "(");

        ast_for->for_node.assignment_expr = parseAssignment(tokens, token_pos);
        demand_token(tokens, token_pos,TOKEN_SYMBOL,";");

        ast_for->for_node.condition_expr = parseExpression(tokens, token_pos);
        demand_token(tokens, token_pos,TOKEN_SYMBOL, ";");

        // To allow "i++" and "i += n" this has to be parseExpression, not only parseReassingment
        ast_for->for_node.reassignment_expr = parseExpression(tokens, token_pos);
        demand_token(tokens, token_pos,TOKEN_SYMBOL, ")");
        demand_token(tokens, token_pos,TOKEN_SYMBOL, "{");

        ast_for->for_node.body = parseBlock(tokens, token_pos);
        demand_token(tokens, token_pos,TOKEN_SYMBOL, "}");
        return ast_for;
    }

    //Foreach node
    else if (tokens[*token_pos]->tk_type == TOKEN_KEYWORD && strcmp(tokens[*token_pos]->tk_value, "foreach") == 0)
    {
        //Create foreach node
        ASTNode *ast_foreach = malloc(sizeof(ASTNode));
        ast_foreach->node_type = NODE_FOREACH;

        //Move past "foreach" and demand "("
        (*token_pos)++;
        demand_token(tokens,token_pos, TOKEN_SYMBOL,"(");
            
        //Parse the declaration of the iterator
        ast_foreach->foreach_node.foreach_declaration = malloc(sizeof(ASTNode));
        ast_foreach->foreach_node.foreach_declaration = parseDeclaration(tokens, token_pos);

        //Demand keyword "in"
        demand_token(tokens, token_pos, TOKEN_KEYWORD, "in");
        //Get the ptr array identifier
        ast_foreach->foreach_node.ptr_array_expr = parseExpression(tokens, token_pos);
        
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
    
    // Extern function definition
    else if (tokens[*token_pos]->tk_type == TOKEN_KEYWORD && strcmp(tokens[*token_pos]->tk_value, "extern") == 0 && 
            strcmp(tokens[*token_pos + 1]->tk_value, "func") == 0)
    {
        // Create extern func node 
        ASTNode *ast_extern_func_def = malloc(sizeof(ASTNode));
        ast_extern_func_def->node_type = NODE_EXTERN_FUNC_DEF;
        ast_extern_func_def->extern_func_def_node.params = malloc(100 * sizeof(Param *));
        ast_extern_func_def->extern_func_def_node.params_count = 0;

        // Advance past 'extern' and 'func'
        (*token_pos) += 2;

        expect_token(tokens, token_pos, TOKEN_KEYWORD, "object");
        expect_token(tokens, token_pos, TOKEN_KEYWORD, "enum");

        if (!(tokens[*token_pos]->tk_type == TOKEN_TYPE || tokens[*token_pos]->tk_type == TOKEN_IDENTIFIER))
        {
            fprintf(stderr, "Expected type after \"func\" keyword.\n");
            exit(1);
        }

        // Check if return type begins with "ptr"
        if (strcmp(tokens[*token_pos]->tk_value, "ptr") == 0)
        {
            ast_extern_func_def->extern_func_def_node.return_type = resolvePtrType(tokens, token_pos);
        }
        else
        {
            // Type is built-in
            ast_extern_func_def->extern_func_def_node.return_type = tokens[*token_pos]->tk_value;
            // Advance past return type
            (*token_pos)++;
        }

        if (tokens[*token_pos]->tk_type != TOKEN_IDENTIFIER)
        {
            fprintf(stderr, "Expected function name after type declaration \"%s\"\n", tokens[*token_pos]->tk_value);
            exit(1);
        }

        ast_extern_func_def->extern_func_def_node.func_name = tokens[*token_pos]->tk_value;
        // printf("We got the function name: %s \n", ast_func_def->funcdef_node.func_name);
        // Advance past function name
        (*token_pos)++;
        // printf("Current token is: %s \n", tokens[*token_pos]->tk_value);
        demand_token(tokens, token_pos, TOKEN_SYMBOL, "(");
        // printf("Passed it \n");
        // Parameters
        while (strcmp(tokens[*token_pos]->tk_value, ")") != 0)
        {

            // Set the param name
            ast_extern_func_def->extern_func_def_node.params[ast_extern_func_def->extern_func_def_node.params_count] = parseFunctionParameter(tokens,token_pos);
            ast_extern_func_def->extern_func_def_node.params_count++;
            expect_token(tokens, token_pos, TOKEN_SYMBOL, ",");
        }

        demand_token(tokens, token_pos, TOKEN_SYMBOL, ")");
        demand_token(tokens, token_pos, TOKEN_SYMBOL, ";");
        return ast_extern_func_def;
    }

    //Function definition
    else if (tokens[*token_pos]->tk_type == TOKEN_KEYWORD &&
             strcmp(tokens[*token_pos]->tk_value, "func") == 0)
    {
        // Create func node
        ASTNode *ast_func_def = malloc(sizeof(ASTNode));
        ast_func_def->funcdef_node.params = malloc(100 * sizeof(Param *));
        ast_func_def->node_type = NODE_FUNC_DEF;
        ast_func_def->funcdef_node.params_count = 0;

        // Advance past func
        (*token_pos)++;

        // Type can be: (object Object) as well as (enum Enum)
        expect_token(tokens, token_pos, TOKEN_KEYWORD, "object");
        expect_token(tokens, token_pos, TOKEN_KEYWORD, "enum");

        if (!(tokens[*token_pos]->tk_type == TOKEN_TYPE || tokens[*token_pos]->tk_type == TOKEN_IDENTIFIER))
        {
            fprintf(stderr, "Expected type after \"func\" keyword.\n");
            exit(1);
        }

        // Check if return type begins with "ptr"
        if (strcmp(tokens[*token_pos]->tk_value, "ptr") == 0)
        {
            ast_func_def->funcdef_node.return_type = resolvePtrType(tokens, token_pos);
        }
        else
        {
            // Type is built-in
            ast_func_def->funcdef_node.return_type = tokens[*token_pos]->tk_value;
            // Advance past return type
            (*token_pos)++;
        }

        if (tokens[*token_pos]->tk_type != TOKEN_IDENTIFIER)
        {
            fprintf(stderr, "Expected function name after type declaration \"%s\"\n", tokens[*token_pos]->tk_value);
            exit(1);
        }

        ast_func_def->funcdef_node.func_name = tokens[*token_pos]->tk_value;
        // printf("We got the function name: %s \n", ast_func_def->funcdef_node.func_name);
        // Advance past function name
        (*token_pos)++;
        // printf("Current token is: %s \n", tokens[*token_pos]->tk_value);
        demand_token(tokens, token_pos, TOKEN_SYMBOL, "(");
        // printf("Passed it \n");
        // Parameters
        while (strcmp(tokens[*token_pos]->tk_value, ")") != 0)
        {

            // Set the param name
            ast_func_def->funcdef_node.params[ast_func_def->funcdef_node.params_count] = parseFunctionParameter(tokens,token_pos);
            ast_func_def->funcdef_node.params_count++;
            expect_token(tokens, token_pos, TOKEN_SYMBOL, ",");
        }

        demand_token(tokens, token_pos, TOKEN_SYMBOL, ")");
        demand_token(tokens, token_pos, TOKEN_SYMBOL, "{");
        ast_func_def->funcdef_node.body = parseBlock(tokens, token_pos);
        demand_token(tokens, token_pos, TOKEN_SYMBOL, "}");
        return ast_func_def;
    }

    //Return statement
    else if (tokens[*token_pos]->tk_type == TOKEN_KEYWORD &&
            strcmp(tokens[*token_pos]->tk_value, "return") == 0)
    {
        // Create return node
        ASTNode *ast_return = malloc(sizeof(ASTNode));
        ast_return->node_type = NODE_RETURN;

        // Advance past 'return'
        (*token_pos)++;

        // Check if there's an expression or if return is void type
        if (strcmp(tokens[*token_pos]->tk_value, ";") == 0)
        {
            ast_return->return_node.return_expr = NULL;
            demand_token(tokens, token_pos, TOKEN_SYMBOL, ";");
            return ast_return;
        }

        ast_return->return_node.return_expr = parseExpression(tokens, token_pos);
        demand_token(tokens, token_pos, TOKEN_SYMBOL, ";");
        return ast_return;
    }

    //System calls
    else if (tokens[*token_pos]->tk_type == TOKEN_SYSCALL)
    {
        ASTNode *ast_syscall = malloc(sizeof(ASTNode));
        ast_syscall->node_type = NODE_SYSCALL;
        ast_syscall->syscall_node.syscall_identifier = tokens[*token_pos]->tk_value;
        (*token_pos)++;
        ast_syscall->syscall_node.operand = parseExpression(tokens, token_pos);
        demand_token(tokens, token_pos, TOKEN_SYMBOL, ";");
        return ast_syscall;
    }
    
    //Callback assignment
    else if (tokens[*token_pos]->tk_type == TOKEN_KEYWORD && strcmp(tokens[*token_pos]->tk_value, "callback") == 0)
    {
        //Create callback node
        ASTNode *ast_cb = malloc(sizeof(ASTNode));
        ast_cb->node_type = NODE_CB_ASSIGNMENT;
        ast_cb->cbassignment_node.param_types = malloc(100 * sizeof(char *));
        ast_cb->cbassignment_node.param_count = 0;
        (*token_pos)++;
        ast_cb->cbassignment_node.cb_identifier = tokens[*token_pos]->tk_value;
        (*token_pos)++;
        demand_token(tokens,token_pos, TOKEN_ASSIGN,"=");
        ast_cb->cbassignment_node.fn_identifider = tokens[*token_pos]->tk_value;
        (*token_pos)++;
        demand_token(tokens,token_pos, TOKEN_SYMBOL,"(");
        while (strcmp(tokens[*token_pos]->tk_value,")") != 0)
        {
            if (strcmp(tokens[*token_pos]->tk_value,"ptr") == 0)
            {
                ast_cb->cbassignment_node.param_types[ast_cb->cbassignment_node.param_count++] = resolvePtrType(tokens, token_pos);
                expect_token(tokens,token_pos, TOKEN_SYMBOL, ",");
                continue;
            }
            ast_cb->cbassignment_node.param_types[ast_cb->cbassignment_node.param_count++] = tokens[*token_pos]->tk_value;
            (*token_pos)++;
            expect_token(tokens,token_pos, TOKEN_SYMBOL, ",");
        }
        
        demand_token(tokens,token_pos, TOKEN_SYMBOL,")");
        demand_token(tokens,token_pos, TOKEN_SYMBOL,";");
        return ast_cb;
    }

    //Enum definition. "enum" + identifier + "{" 
    else if (tokens[*token_pos]->tk_type == TOKEN_KEYWORD && strcmp(tokens[*token_pos]->tk_value, "enum") == 0
            && tokens[*token_pos + 1]->tk_type == TOKEN_IDENTIFIER && strcmp(tokens[*token_pos + 2]->tk_value, "{") == 0)
    {
        //Skip the enum keyword
        (*token_pos)++;
        ASTNode *ast_enum = malloc(sizeof(ASTNode));
        ast_enum->node_type = NODE_ENUM;
        
        //Get the identifier
        ast_enum->enum_node.identifier = tokens[*token_pos]->tk_value;
        (*token_pos)++;

        demand_token(tokens, token_pos, TOKEN_SYMBOL, "{");

        //Get all the "declarations". Only 255 allowed, using uint8 as type 
        ast_enum->enum_node.declaration_count = 0;
        ast_enum->enum_node.declarations = malloc(255 * sizeof(ASTNode *));

        while (strcmp(tokens[*token_pos]->tk_value, "}") != 0)
        {
            //Create the "declaration" node
            //enum MyEnum {  FIRST, SECOND, THIRD,...    }  = enum MyEnum {  MyEnum FIRST = 0, MyEnum SECOND = 1,...  }
            ASTNode *ast_enum_decl = malloc(sizeof(ASTNode));
            ast_enum_decl->node_type = NODE_ASSIGNMENT;
            ast_enum_decl->assignment_node.type = ast_enum->enum_node.identifier;
            ast_enum_decl->assignment_node.identifier = tokens[*token_pos]->tk_value;
            (*token_pos)++;
            ASTNode *ast_number = malloc(sizeof(ASTNode));
            ast_number->node_type = NODE_NUMBER;
            ast_number->number_node.number_value = ast_enum->enum_node.declaration_count;
            ast_enum_decl->assignment_node.expression = ast_number;

            ast_enum->enum_node.declarations[ast_enum->enum_node.declaration_count++] = ast_enum_decl;
            demand_token(tokens, token_pos, TOKEN_SYMBOL, ",");
        }
        
        demand_token(tokens, token_pos, TOKEN_SYMBOL, "}");
        return ast_enum;
    }

    //Object definition. "object" + identifier + ("inherits" + identifier)? + {
    else if (tokens[*token_pos]->tk_type == TOKEN_KEYWORD &&
            (strcmp(tokens[*token_pos]->tk_value, "object") == 0) &&
            ( strcmp(tokens[*token_pos + 2]->tk_value, "{") == 0 || 
             strcmp(tokens[*token_pos + 2]->tk_value,"inherits") == 0 ))
    {
        // Create object node
        ASTNode *ast_object = malloc(sizeof(ASTNode));
        ast_object->node_type = NODE_OBJECT;
        

        // Move past the 'object' keyword. Get the object name
        (*token_pos)++;
        ast_object->object_node.identifier = tokens[*token_pos]->tk_value;
        ast_object->object_node.parent = NULL;  
        // Move past the identifier
        (*token_pos)++;
                    
        // Check whether current object definition inherits another one
        if (expect_token(tokens, token_pos, TOKEN_KEYWORD, "inherits"))
        {
            ast_object->object_node.parent = tokens[*token_pos]->tk_value;
            (*token_pos)++;
        }

        demand_token(tokens, token_pos, TOKEN_SYMBOL, "{");

        // Allocate for block of declarations
        ast_object->object_node.declaration_count = 0;
        ast_object->object_node.declarations = malloc(100 * sizeof(ASTNode *));

        while (strcmp(tokens[*token_pos]->tk_value, "}") != 0)
        {
            // 23/11 - Reallocate enough memory to allow more declarations. maybe another day holy shit


            ast_object->object_node.declarations[ast_object->object_node.declaration_count] = parseDeclaration(tokens, token_pos);
            ast_object->object_node.declaration_count++;
            demand_token(tokens, token_pos, TOKEN_SYMBOL, ";");
        }

        demand_token(tokens, token_pos, TOKEN_SYMBOL, "}");
        return ast_object;
    }
<<<<<<< Updated upstream

    //Ptr field reassignment   "(" "*" ptr_id ")"
    else if (strcmp(tokens[*token_pos]->tk_value, "(") == 0 && strcmp(tokens[*token_pos + 1]->tk_value, "*" ) == 0)
    {
        (*token_pos) += 2;
        ASTNode *ptr_field = malloc(sizeof(ASTNode));
        ptr_field->node_type = NODE_PTR_FIELDREASSIGNMENT;

        ptr_field->ptr_fieldreassign_node.ptr_identifier = tokens[*token_pos]->tk_value;
        (*token_pos)++;
        demand_token(tokens, token_pos, TOKEN_SYMBOL, ")");
        demand_token(tokens, token_pos, TOKEN_SYMBOL, ".");
        ptr_field->ptr_fieldreassign_node.field_identifier = tokens[*token_pos]->tk_value;
        (*token_pos)++;

        ptr_field->ptr_fieldreassign_node.op = tokens[*token_pos]->tk_type;
        (*token_pos)++;

        ptr_field->ptr_fieldreassign_node.expression = parseExpression(tokens, token_pos);
        demand_token(tokens, token_pos, TOKEN_SYMBOL, ";");

        return ptr_field;
    }

    // Assignments.
    //assignment  := ( ( "object"? identifier ) | builtinType | ptr "<" ("object"? identifier) | builtinType ">" ) identifier  "=" expression ";"
    // type         := "i32" | "str" | "char" | "bool" | "ptr" | ("object" identifier)
    //Pointer declaration, assignment or reassignment
    else if (strcmp(tokens[*token_pos]->tk_value, "*") == 0 || strcmp(tokens[*token_pos]->tk_value, "ptr") == 0)
    {
        ASTNode *node = parsePtrAssignment(tokens, token_pos);
        demand_token(tokens, token_pos,TOKEN_SYMBOL, ";");
        return node;
    }

    else if (tokens[*token_pos]->tk_type == TOKEN_TYPE || strcmp(tokens[*token_pos]->tk_value, "object") == 0
    ||  strcmp(tokens[*token_pos]->tk_value, "enum") == 0  ||tokens[*token_pos]->tk_type == TOKEN_IDENTIFIER )
=======

    // Assignments.
    //assignment := type identifier "=" expression ";"
    else if (is_assignment(tokens, *token_pos))
>>>>>>> Stashed changes
    {
        ASTNode *node = parseAssignment(tokens, token_pos);
        demand_token(tokens, token_pos,TOKEN_SYMBOL, ";");
        return node;
    }

    // Declaration
    else if (is_declaration(tokens, *token_pos) )
    {
        ASTNode *node = malloc(sizeof(ASTNode));
        node->node_type = NODE_DECLARATION;
        node = parseDeclaration(tokens, token_pos);

        return node;
    }
    
    // Reassignment 
    else if (is_reassignment(tokens, *token_pos))
    {
        ASTNode *node = parseReassignment(tokens, token_pos);
        demand_token(tokens, token_pos, TOKEN_SYMBOL, ";");
        return node;
    }

    // Assume its an expression for now 
    ASTNode *expr = parseExpression(tokens, token_pos);
    expr->node_type = NODE_STDALONE_POSTFIX_OP;
    demand_token(tokens, token_pos, TOKEN_SYMBOL, ";");
    return expr;


    // else
    // {
    //     fprintf(stderr, "Unsupported token type when parsing statement: %s '%s' \n",
    //             tokenTypeToStr(tokens[*token_pos]->tk_type), tokens[*token_pos]->tk_value);
    //     exit(1);
    // }
}

Param *parseFunctionParameter(Token **tokens, int *token_pos)
{

    //Starting with identifier + "(" --> function parameter
    if (tokens[*token_pos]->tk_type == TOKEN_IDENTIFIER && strcmp(tokens[*token_pos + 1]->tk_value, "(") == 0)
    {
        //printf("Parsing parameter with supertype function.\n");
        //Create a Param 
        Param *param = malloc(sizeof(Param));
        param->supertype = PARAM_FUNCTION;
        param->functionParam.params_type = malloc(100 * sizeof(char *));
        param->functionParam.identifier = tokens[*token_pos]->tk_value;
        param->functionParam.type = "callback";
        param->functionParam.params_count = 0;
        (*token_pos)++;
        demand_token(tokens,token_pos, TOKEN_SYMBOL, "(");
        while (strcmp(tokens[*token_pos]->tk_value,")") != 0)
        {
            expect_token(tokens,token_pos,TOKEN_KEYWORD, "object");
            expect_token(tokens, token_pos, TOKEN_KEYWORD, "enum");

            if (strcmp(tokens[*token_pos]->tk_value, "ptr") == 0)
            {
                param->functionParam.params_type[param->functionParam.params_count++] = resolvePtrType(tokens, token_pos);
            }
            else
            {
                param->functionParam.params_type[param->functionParam.params_count++] = tokens[*token_pos]->tk_value;
                (*token_pos)++;
            }
            expect_token(tokens,token_pos, TOKEN_SYMBOL, ",");
        }

        demand_token(tokens,token_pos, TOKEN_SYMBOL, ")");
        demand_token(tokens,token_pos, TOKEN_ASSIGN, "=");
        demand_token(tokens,token_pos, TOKEN_OPERATOR, ">");
        param->functionParam.return_type = tokens[*token_pos]->tk_value;
        (*token_pos)++;
        //printf("Finished parsing parameter with supertype function.\n");
        return param;
    }

    //Else assume is immediate
    Param *param = malloc(sizeof(Param));
    param->supertype = PARAM_IMMEDIATE;
    if (strcmp(tokens[*token_pos]->tk_value, "ptr") == 0)
    {
        param->immediateParam.type = resolvePtrType(tokens,token_pos);
    }
    else
    {
        expect_token(tokens,token_pos,TOKEN_KEYWORD, "object");
        expect_token(tokens, token_pos, TOKEN_KEYWORD, "enum");
        param->immediateParam.type = tokens[*token_pos]->tk_value;
        (*token_pos)++;
    }
    param->immediateParam.param_name = tokens[*token_pos]->tk_value;
    (*token_pos)++;
    return param;
}

ASTNode *parseUseDirective(Token **tokens,int *token_pos)
{
    ASTNode *ast_node = malloc(sizeof(ASTNode ));
    ast_node->node_type = NODE_USE_DIRECTIVE;
    //Advance past # and "use"
    (*token_pos) += 2;
    //Get the file path
   
    if (is_already_used(tokens[*token_pos]->tk_value))
    {
        ast_node->node_type = NODE_NULL;
        (*token_pos)++;
        return ast_node;
    }

    add_used_file(tokens[*token_pos]->tk_value);
    ast_node->use_node.filepath = tokens[*token_pos]->tk_value;
    //Advance past the string
    (*token_pos) += 1;

    char *src = read_file(ast_node->use_node.filepath);
    Token **file_tokens = tokenize(src);
    //print_tokens(file_tokens);
    int file_tokens_pos = 0;

    ast_node->use_node.program = parseProgram(file_tokens, &file_tokens_pos);

    return ast_node;
}


ASTNode *parsePtrAssignment(Token **tokens, int *token_pos)
{
    //Check if the current token is "*", in that case, pointer reassignment.
    if (strcmp(tokens[*token_pos]->tk_value, "*") == 0 && tokens[*token_pos]->tk_type == TOKEN_OPERATOR)
    {
        //Expression is somewhat like **(ptr + *ptr2 + getOffset()  ). Has to resolve to a memory location
        // Count the number of consecutive "*" operators (dereference levels)
        int deref_levels = 0;
        while (strcmp(tokens[*token_pos]->tk_value, "*") == 0 && tokens[*token_pos]->tk_type == TOKEN_OPERATOR)
        {
            deref_levels++;
            (*token_pos)++;
        }
        //Parse the left value node.
        
        ASTNode *lvalue_node = parseExpression(tokens, token_pos);

        expect_token(tokens, token_pos, TOKEN_SYMBOL, ")");
        //Create the ptr reassignment node
        ASTNode *ptr_reassignment_node = malloc(sizeof(ASTNode ));
        ptr_reassignment_node->node_type = NODE_PTR_REASSIGNMENT;
        //ptr_reassignment_node->ptr_reassignment_node.ptr_identifier = tokens[token_pos]->tk_value;
        ptr_reassignment_node->ptr_reassignment_node.deref_level = deref_levels;
        ptr_reassignment_node->ptr_reassignment_node.lvalue = lvalue_node;
        //Advance past identifier
       
        ptr_reassignment_node->ptr_reassignment_node.op = tokens[*token_pos]->tk_type;
        (*token_pos)++;
         
        ptr_reassignment_node->ptr_reassignment_node.expression = parseExpression(tokens, token_pos);
        return ptr_reassignment_node;
    }

<<<<<<< Updated upstream
    //Pointer type case. Could be declaration or assignment
    if (strcmp(tokens[*token_pos]->tk_value, "ptr") == 0)
    {

        char *total_type = resolvePtrType(tokens, token_pos);

        //Get the identifier
        char *identifier = tokens[*token_pos]->tk_value;
        //Advance past identifier
=======
// Function to resolve the type 
char *tryResolveType(Token **tokens, int *token_pos)
{

     char *typeBuffer = malloc(256);
    memset(typeBuffer, 0, 256);

    if (strcmp(tokens[*token_pos]->tk_value, "ptr") == 0)
    {
        // Handle pointer type recursively
        char *ptrType = resolvePtrType(tokens, token_pos);
        strcat(typeBuffer, ptrType);
        free(ptrType);
    }
    else if (tokens[*token_pos]->tk_type == TOKEN_TYPE || tokens[*token_pos]->tk_type == TOKEN_IDENTIFIER)
    {
        strcat(typeBuffer, tokens[*token_pos]->tk_value);
        (*token_pos)++;
    }
    else
    {
        fprintf(stderr, "Unexpected token when resolving type: '%s'\n", tokens[*token_pos]->tk_value);
        exit(1);
    }

    // Handle optional array [size]
    if (tokens[*token_pos]->tk_type == TOKEN_SYMBOL && strcmp(tokens[*token_pos]->tk_value, "[") == 0)
    {
        (*token_pos)++;
        if (tokens[*token_pos]->tk_type != TOKEN_NUMBER)
        {
            fprintf(stderr, "Array size must be a number, got '%s'\n", tokens[*token_pos]->tk_value);
            exit(1);
        }

        strcat(typeBuffer, " [");
        strcat(typeBuffer, tokens[*token_pos]->tk_value);
        strcat(typeBuffer, "]");

        (*token_pos)++;
        // Expect closing ']'
        if (tokens[*token_pos]->tk_type != TOKEN_SYMBOL || strcmp(tokens[*token_pos]->tk_value, "]") != 0)
        {
            fprintf(stderr, "Expected ']' after array size, got '%s'\n", tokens[*token_pos]->tk_value);
            exit(1);
        }
        (*token_pos)++;
    }

    return typeBuffer;
    
}


// Function to resolve the type 
char *resolveType(Token **tokens, int *token_pos)
{

     char *typeBuffer = malloc(256);
    memset(typeBuffer, 0, 256);

    if (strcmp(tokens[*token_pos]->tk_value, "ptr") == 0)
    {
        // Handle pointer type recursively
        char *ptrType = resolvePtrType(tokens, token_pos);
        strcat(typeBuffer, ptrType);
        free(ptrType);
    }
    else if (tokens[*token_pos]->tk_type == TOKEN_TYPE || tokens[*token_pos]->tk_type == TOKEN_IDENTIFIER)
    {
        strcat(typeBuffer, tokens[*token_pos]->tk_value);
>>>>>>> Stashed changes
        (*token_pos)++;
    }
    else
    {
        fprintf(stderr, "Unexpected token when resolving type: '%s'\n", tokens[*token_pos]->tk_value);
        exit(1);
    }

<<<<<<< Updated upstream
        //Check whether it is assignment or declaration
        if ( expect_token(tokens, token_pos, TOKEN_ASSIGN, "=") == 1)
        {

            //Create the assignment node
            ASTNode *ast_assignment = malloc(sizeof(ASTNode ));
            ast_assignment->node_type = NODE_ASSIGNMENT;
            ast_assignment->assignment_node.type = total_type;
            ast_assignment->assignment_node.identifier = identifier;
            ast_assignment->assignment_node.expression = parseExpression(tokens, token_pos);
            return ast_assignment;
        }

        //Declaration
        else if (strcmp(tokens[*token_pos]->tk_value, ";") == 0)
        {
            //Create the declaration node
            ASTNode *ast_declaration_node = malloc(sizeof(ASTNode ));
            ast_declaration_node->node_type = NODE_DECLARATION;
            ast_declaration_node->declaration_node.type = total_type;
            ast_declaration_node->declaration_node.identifier = identifier;

            return ast_declaration_node;
        }

        else
        {
            fprintf(stderr, "Unexpected token while parsing statement starting with \"ptr\": %s \n", tokens[*token_pos]->tk_value);
            exit(1);
        }

    }
=======
    // Handle optional array [size]
    if (tokens[*token_pos]->tk_type == TOKEN_SYMBOL && strcmp(tokens[*token_pos]->tk_value, "[") == 0)
    {
        (*token_pos)++;
        if (tokens[*token_pos]->tk_type != TOKEN_NUMBER)
        {
            fprintf(stderr, "Array size must be a number, got '%s'\n", tokens[*token_pos]->tk_value);
            exit(1);
        }

        strcat(typeBuffer, " [");
        strcat(typeBuffer, tokens[*token_pos]->tk_value);
        strcat(typeBuffer, "]");

        (*token_pos)++;
        // Expect closing ']'
        if (tokens[*token_pos]->tk_type != TOKEN_SYMBOL || strcmp(tokens[*token_pos]->tk_value, "]") != 0)
        {
            fprintf(stderr, "Expected ']' after array size, got '%s'\n", tokens[*token_pos]->tk_value);
            exit(1);
        }
        (*token_pos)++;
    }

    return typeBuffer;
    
}
>>>>>>> Stashed changes

    else
    {
        fprintf(stderr, "Unexpected token while parsing statement ptr assignment: %s \n", tokens[*token_pos]->tk_value);
        exit(1);
    }
}

// Function to resolve the type of a pointer
char *resolvePtrType(Token **tokens, int *token_pos)
{
    // Allocate a buffer to store the resolved type
    char *type = malloc(100 * sizeof(char));
    if (type == NULL)
    {
        fprintf(stderr, "Memory allocation failed\n");
        return NULL;
    }
    type[0] = '\0'; // Initialize the buffer as an empty string

    // Loop through the tokens
    while (tokens[*token_pos] != NULL)
    {
        if (strcmp(tokens[*token_pos]->tk_value, "ptr") == 0)
        {
            // Append "ptr" to the type
            strcat(type, "ptr ");
            (*token_pos)++; // Move to the next token

            // Check if the next token is '<'
            if (tokens[*token_pos]->tk_value != NULL && strcmp(tokens[*token_pos]->tk_value, "<") == 0)
            {
                (*token_pos)++; // Move past '<'
                // Recursively resolve the inner type
                char *inner_type = resolvePtrType(tokens, token_pos);
                if (inner_type == NULL)
                {
                    free(type);
                    return NULL;
                }
                // Append the inner type to the current type
                strcat(type, inner_type);
                free(inner_type); // Free the inner type after appending
            }
            else
            {
                // If there's no '<', this is a base type (e.g., "ptr int16")
                break;
            }

            // Check if the next token is '>'
            if (tokens[*token_pos]->tk_value != NULL && strcmp(tokens[*token_pos]->tk_value, ">") == 0)
            {
                (*token_pos)++; // Move past '>'
                return type;
            }
            else
            {
                // Handle syntax error (missing '>')
                fprintf(stderr, "Syntax error: Expected '>' while resolving ptr inner type. \n");
                free(type);
                return NULL;
            }
        }
        else
        {
            // Handle base types (e.g., "int16", "char", "bool", "object OBJECTNAME", "OBJECTNAME")
            // Append the base type to the resolved type
            // Skip the object optional keyword
            if (strcmp(tokens[*token_pos]->tk_value, "object") == 0) { (*token_pos)++; }
            // Skip the optional enum keyword 
            if (strcmp(tokens[*token_pos]->tk_value, "enum") == 0) { (*token_pos)++; }
            strcat(type, tokens[*token_pos]->tk_value);
            
            (*token_pos)++; // Move to the next token
            break; // Base type reached, stop recursion
        }
    }

    return type;
}

<<<<<<< Updated upstream
//TODO: SIMPLIFY THIS?
//assignment  := ( ( "object"? identifier ) | builtinType | ptr "<" ("object"? identifier) | builtinType ">" ) identifier  "=" expression ";"
=======
ASTNode *parseReassignment(Token **tokens, int *token_pos)
{
    printf("Parsing a reassingment with current token = %s \n", tokens[*token_pos]->tk_value);
    ASTNode *node = malloc(sizeof(ASTNode));
    node->node_type = NODE_REASSIGNMENT;
    printf("About to parse LValue\n");
    node->reassignment_node.lvalue = parseLValue(tokens, token_pos);

    printf("About to parse RValue \n");
    node->reassignment_node.line_number = tokens[*token_pos]->line_number;
    node->reassignment_node.op = tokens[*token_pos]->tk_type;
    *(token_pos) += 1;
    node->reassignment_node.expression = parseExpression(tokens, token_pos);

    printf("Finished\n");
    return node;
}

// assignment = type lvalue '=' expr;
// type = object? identifier | enum? identifier | built-in type 
>>>>>>> Stashed changes
ASTNode *parseAssignment(Token **tokens, int *token_pos)
{
    //This function is called whenever we encounter a TYPE, "object" or IDENTIFIER and no other case is possible

    //Be within len(tokens)
    if (tokens[*token_pos + 1]->tk_type == TOKEN_EOF || tokens[*token_pos + 2]->tk_type == TOKEN_EOF)
    {
        fprintf(stderr, "End of file reached before end of assignment. \n");
        exit(1);
    }

    expect_token(tokens, token_pos, TOKEN_KEYWORD, "object");
    expect_token(tokens, token_pos, TOKEN_KEYWORD, "enum");
    //Reassignment of instance field
    if (tokens[*token_pos]->tk_type == TOKEN_IDENTIFIER && strcmp(tokens[*token_pos + 1]->tk_value, ".") == 0 &&
        tokens[*token_pos + 2]->tk_type == TOKEN_IDENTIFIER)
    {

        //Instance node
        ASTNode *ast_instance = malloc(sizeof(ASTNode));
        ast_instance->node_type = NODE_INSTANCE_REASSIGNMENT;
        ast_instance->field_reassign_node.instance_identifier = tokens[*token_pos]->tk_value;

        //Advance past object instance identifier + "."
        (*token_pos) += 2;
        ast_instance->field_reassign_node.field_identifier = tokens[*token_pos]->tk_value;
        //Advance past field name and "="
        (*token_pos)++;

        ast_instance->field_reassign_node.op = tokens[*token_pos]->tk_type;
        (*token_pos)++;

        ast_instance->field_reassign_node.expression = parseExpression(tokens, token_pos);
        return ast_instance;
    }
<<<<<<< Updated upstream
 
    // Check if the current token is a type (built-in or objectIdentifier)
    int is_type = (tokens[*token_pos]->tk_type == TOKEN_TYPE || tokens[*token_pos]->tk_type == TOKEN_IDENTIFIER) && 
                    tokens[*token_pos + 1]->tk_type == TOKEN_IDENTIFIER;
    if (!is_type)
=======

    // At this point we have an identifier | type 
    ASTNode *ast_assignment = malloc(sizeof(ASTNode));
    ast_assignment->node_type = NODE_ASSIGNMENT;
    

    ast_assignment->assignment_node.type = resolveType(tokens, token_pos);

    printf("########### WE RESOLVED A TYPE: %s \n", ast_assignment->assignment_node.type);

    // Need an identifier for the variable 
    if (tokens[*token_pos]->tk_type != TOKEN_IDENTIFIER)
>>>>>>> Stashed changes
    {
        // If not a type, it must be a reassignment
        if (tokens[*token_pos]->tk_type == TOKEN_IDENTIFIER && is_assignable_op(tokens,token_pos,1))
        {
            // Reassignment case
            ASTNode *ast_reassignment = malloc(sizeof(ASTNode));
            ast_reassignment->node_type = NODE_REASSIGNMENT;
            ast_reassignment->reassignment_node.identifier = tokens[*token_pos]->tk_value;
            (*token_pos)++; // Advance past the identifier
            
            ast_reassignment->reassignment_node.op = tokens[*token_pos]->tk_type;
            (*token_pos)++;

            ast_reassignment->reassignment_node.expression = parseExpression(tokens, token_pos);
            return ast_reassignment;
        }

        else
        {
            fprintf(stderr, "Expected type or identifier for assignment. Got: %s\n", tokens[*token_pos]->tk_value);
            exit(1);
        }
    }
   
    // At this point, we have a type (built-in or objectIdentifier) followed by an identifier
    char *type = tokens[*token_pos]->tk_value; // ObjectType or built-in type
    (*token_pos)++; // Advance past the type

    // Ensure the next token is an identifier
    if (tokens[*token_pos]->tk_type != TOKEN_IDENTIFIER)
    {
        fprintf(stderr, "Expected identifier after type: %s\n", type);
        exit(1);
    }


    char *identifier = tokens[*token_pos]->tk_value; // Identifier
    (*token_pos)++; // Advance past the identifier

    // Check if this is an assignment or a declaration
    if (tokens[*token_pos]->tk_type == TOKEN_ASSIGN && strcmp(tokens[*token_pos]->tk_value, "=") == 0)
    {
        // Assignment case
        ASTNode *ast_assignment = malloc(sizeof(ASTNode));
        ast_assignment->node_type = NODE_ASSIGNMENT;
        ast_assignment->assignment_node.type = type;
        ast_assignment->assignment_node.identifier = identifier;

        (*token_pos)++; // Advance past the "="
        ast_assignment->assignment_node.expression = parseExpression(tokens, token_pos);
        return ast_assignment;
    }

<<<<<<< Updated upstream
    else
    {
        // Declaration case
        // Rewind to the type token so parseDeclaration can handle it
        (*token_pos) -= 2; // Go back to the type token
        // Delegate to parseDeclaration
        return parseDeclaration(tokens, token_pos);
    }
}

ASTNode *parseDeclaration(Token **tokens,int *token_pos)
{
    // Check if the current token is "object" (optional) or "enum"
    expect_token(tokens, token_pos, TOKEN_KEYWORD, "object");
    expect_token(tokens, token_pos, TOKEN_KEYWORD, "enum");
        // Ensure the next token is a type (built-in or objectIdentifier)
    if (tokens[*token_pos]->tk_type == TOKEN_TYPE || tokens[*token_pos]->tk_type == TOKEN_IDENTIFIER)
    {
        ASTNode *ast_declaration = malloc(sizeof(ASTNode));
        ast_declaration->node_type = NODE_DECLARATION;

        //ptr type
        if (strcmp(tokens[*token_pos]->tk_value, "ptr") == 0)
=======

    if (strcmp(tokens[*token_pos]->tk_value,";") == 0)
    {
        ASTNode *ast_declaration = malloc(sizeof(ASTNode));
        ast_declaration->node_type = NODE_DECLARATION;
        ast_declaration->declaration_node.type = ast_assignment->assignment_node.type;
        ast_declaration->declaration_node.identifier = ast_assignment->assignment_node.identifier;
        printf("WE ARE HERE FOR TYPE : %s \n", ast_declaration->declaration_node.type);


        free(ast_assignment);
        return ast_declaration;
    }
}

// Declaration  := type identifier ';'
// type = object? identifier | enum? identifier | built-in type 
ASTNode *parseDeclaration(Token **tokens,int *token_pos)
{
    printf("PARSING A DECLARATION \n");

    // Skip optinals keywords "object" "enum"
    expect_token(tokens, token_pos, TOKEN_KEYWORD, "object");
    expect_token(tokens, token_pos, TOKEN_KEYWORD, "enum");

    // Ensure the next token is a type (built-in or objectIdentifier)
    if (tokens[*token_pos]->tk_type != TOKEN_TYPE && tokens[*token_pos]->tk_type != TOKEN_IDENTIFIER)
    {
        fprintf(stderr, "Expected type (Identifier | built-in type) in declaration.\n");
        exit(1);
    }

    
    // At this point we have an identifier | type 
    ASTNode *ast_declaration = malloc(sizeof(ASTNode));
    ast_declaration->node_type = NODE_DECLARATION;


    ast_declaration->declaration_node.type = resolveType(tokens, token_pos);
    //(*token_pos)++;

    printf("Current token is %s \n", tokens[*token_pos]->tk_value);


    // Need an identifier for the variable 
    if (tokens[*token_pos]->tk_type != TOKEN_IDENTIFIER)
    {
        fprintf(stderr, "Expected variable identifier in declaration.\n");
        exit(1);
    }

    // At this point we have an identifier for the variable 
    ast_declaration->declaration_node.identifier = tokens[*token_pos]->tk_value;

    (*token_pos)++;
    
    return ast_declaration;


}

ASTNode *parseArrayInit(Token **tokens, int *token_pos, char *arr_name)
{
    ASTNode *ast_array = malloc(sizeof(ASTNode));
    ast_array->node_type = NODE_ARRAY_INIT;
    ast_array->array_init_node.capacity = 100;
    ast_array->array_init_node.elements = malloc(sizeof(ASTNode *) * ast_array->array_init_node.capacity);
    ast_array->array_init_node.arr_name = arr_name;
    ast_array->array_init_node.size = 0;

    while (tokens[*token_pos]->tk_type != TOKEN_EOF && strcmp(tokens[*token_pos]->tk_value, "]"))
    {
        if (ast_array->array_init_node.size + 1 > ast_array->array_init_node.capacity)
>>>>>>> Stashed changes
        {
            ast_declaration->declaration_node.type = resolvePtrType(tokens, token_pos);
        }
        else
        {
            ast_declaration->declaration_node.type = tokens[*token_pos]->tk_value; // ObjectType or built-in type
            (*token_pos)++; // Move past the type
        }
        // Ensure the next token is an identifier
        if (tokens[*token_pos]->tk_type != TOKEN_IDENTIFIER)
        {
            fprintf(stderr, "Expected identifier after type: %s\n", tokens[*token_pos - 1]->tk_value);
            exit(1);
        }

        ast_declaration->declaration_node.identifier = tokens[*token_pos]->tk_value; // Identifier
        (*token_pos)++; // Move past the identifier
        return ast_declaration;
    }

    else
    {
        fprintf(stderr, "Invalid token in declaration. Got: %s\n", tokens[*token_pos]->tk_value);
        exit(1);
    }

}

//Expression handles '||'. Precedence 6.
ASTNode *parseExpression(Token **tokens, int *token_pos)
{
    //Empty expression
    if (strcmp(tokens[*token_pos]->tk_value, ";") == 0)
    {
        ASTNode *ast_empty = malloc(sizeof(ASTNode));
        ast_empty->node_type = NODE_NULL;
        return ast_empty;
    }

<<<<<<< Updated upstream
    ASTNode *left = parseExpr_Precedence5(tokens, token_pos);
=======
    // Reassignment 
    if (is_reassignment(tokens, *token_pos))
    {
        ASTNode *ast_reassignment = parseReassignment(tokens, token_pos);
        ast_reassignment->node_type = NODE_REASSIGNMENT;
        return ast_reassignment;
    }



    ASTNode *left = parseExpr_Precedence7(tokens, token_pos);
>>>>>>> Stashed changes

    while (tokens[*token_pos]->tk_type == TOKEN_OPERATOR &&
           (strcmp(tokens[*token_pos]->tk_value, "||") == 0))
    {

        ASTNode *ast_bin_op = malloc(sizeof(ASTNode ));
        ast_bin_op->node_type = NODE_BINARY_OP;
        ast_bin_op->binary_op_node.op = tokens[*token_pos]->tk_value;
        //Advance to the next token after operator
        (*token_pos)++;

        //Create the right and binary op node
        ASTNode *right = parseExpr_Precedence5(tokens, token_pos);

        ast_bin_op->binary_op_node.right = right;
        ast_bin_op->binary_op_node.left = left;

        /*
        if (left->node_type == NODE_NUMBER && right->node_type == NODE_NUMBER)
        {

            int64_t lnumber = left->number_node.number_value;
            int64_t rnumber = right->number_node.number_value;

            int64_t result = foldBinOperation(lnumber, rnumber, ast_bin_op->binary_op_node.op);

            ASTNode *ast_number = malloc(sizeof(ASTNode));
            ast_number->node_type = NODE_NUMBER;
            ast_number->number_node.number_value = result;

            return ast_number;
        }
        */

        //Maybe there's only a left
        left = ast_bin_op;
    }

    return left;
}


//Expression handles '&&'. Precedence 4.
ASTNode *parseExpr_Precedence5(Token **tokens, int *token_pos)
{
    ASTNode *left = parseExpr_Precedence4(tokens, token_pos);

    while (tokens[*token_pos]->tk_type == TOKEN_OPERATOR &&
        (strcmp(tokens[*token_pos]->tk_value, "&&") == 0))
    {

        ASTNode *ast_bin_op = malloc(sizeof(ASTNode ));
        ast_bin_op->node_type = NODE_BINARY_OP;
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

//Expr handles "!=", "==". Precedence 4
ASTNode *parseExpr_Precedence4(Token **tokens, int *token_pos)
{
    //Left node comes from parsing the term
    ASTNode *left = parseExpr_Precedence3(tokens, token_pos);

    while ( tokens[*token_pos]->tk_type == TOKEN_OPERATOR &&
            ( (strcmp(tokens[*token_pos]->tk_value, "!=") == 0)))

    {


        ASTNode *ast_bin_op = malloc(sizeof(ASTNode ));
        ast_bin_op->node_type = NODE_BINARY_OP;
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

//Expression handles '>','<', "<=",">=". Precedence 3.
ASTNode *parseExpr_Precedence3(Token **tokens, int *token_pos)
{
    //Left node comes from parsing the term
    ASTNode *left = parseExpr_Precedence2(tokens, token_pos);

    while ( tokens[*token_pos]->tk_type == TOKEN_OPERATOR &&
            (  (strcmp(tokens[*token_pos]->tk_value, ">") == 0) ||
            (strcmp(tokens[*token_pos]->tk_value, "<") == 0) ||
            (strcmp(tokens[*token_pos]->tk_value,"<=") == 0)  ||
            (strcmp(tokens[*token_pos]->tk_value, ">=") == 0)  ||
            (strcmp(tokens[*token_pos]->tk_value, "==") == 0 )))

    {


        ASTNode *ast_bin_op = malloc(sizeof(ASTNode ));
        ast_bin_op->node_type = NODE_BINARY_OP;
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

//Expression handles '+' and '-'. Precedence 2.
ASTNode *parseExpr_Precedence2(Token **tokens, int *token_pos)
{
    //Left node comes from parsing the term
    ASTNode *left = parseExpr_Precedence1(tokens, token_pos);

    while (tokens[*token_pos]->tk_type == TOKEN_OPERATOR &&
           (strcmp(tokens[*token_pos]->tk_value, "+") == 0 ||
            strcmp(tokens[*token_pos]->tk_value, "-") == 0))
    {


        ASTNode *ast_bin_op = malloc(sizeof(ASTNode ));
        ast_bin_op->node_type = NODE_BINARY_OP;
        ast_bin_op->binary_op_node.op = tokens[*token_pos]->tk_value;
        //Advance to the next token after operator
        (*token_pos)++;

        //Create the right and binary op node
        ASTNode *right = parseExpr_Precedence1(tokens, token_pos);

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


//Expression handles "*" and "/". Precedence 1.
ASTNode *parseExpr_Precedence1(Token **tokens, int *token_pos)
{
    //Left node comes from parsing the term
    ASTNode *left = parseUnit(tokens, token_pos);

    while (tokens[*token_pos]->tk_type == TOKEN_OPERATOR &&
           (strcmp(tokens[*token_pos]->tk_value, "*") == 0 ||
            strcmp(tokens[*token_pos]->tk_value, "/") == 0 ||
            strcmp(tokens[*token_pos]->tk_value, "%") == 0))
    {


        ASTNode *ast_bin_op = malloc(sizeof(ASTNode ));
        ast_bin_op->node_type = NODE_BINARY_OP;
        ast_bin_op->binary_op_node.op = tokens[*token_pos]->tk_value;
        //Advance to the next token after operator
        (*token_pos)++;

        //Create the right and binary op node
        ASTNode *right = parseUnit(tokens, token_pos);

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

<<<<<<< Updated upstream
=======
// Unary opeartions, only * allowed as lvalue
ASTNode *parseLValue_Precedence2(Token **tokens, int *token_pos)
{
    printf("PARSING LVALUE PRECEDENCE 2\n");
    ASTNode *operand = NULL;

    while (tokens[*token_pos]->tk_type == TOKEN_OPERATOR &&
           (strcmp(tokens[*token_pos]->tk_value, "*") == 0))
    {
         //Create a Unary Node
        ASTNode *ast_unary = malloc(sizeof(ASTNode ));
        ast_unary->node_type = NODE_UNARY_OP;
        ast_unary->unary_op_node.op = tokens[*token_pos]->tk_value;
        (*token_pos)++;
        ast_unary->unary_op_node.right = parseExpr_Precedence1(tokens, token_pos);
        
        ASTNode *ast_number = tryFoldUnary(ast_unary->unary_op_node.right, ast_unary->unary_op_node.op);

        if (ast_number != NULL)
        {
            operand = ast_number;
            continue;
        }

        operand = ast_unary;
    }

    if (operand != NULL) { return operand; }

    return parseExpr_Precedence1(tokens, token_pos);

}

// Unary operations (unary -, address &, dereference * ). Precedence 2
ASTNode *parseExpr_Precedence2(Token **tokens, int *token_pos)
{
   
    ASTNode *operand = NULL;

    while (tokens[*token_pos]->tk_type == TOKEN_OPERATOR &&
           (strcmp(tokens[*token_pos]->tk_value, "*") == 0 ||
            strcmp(tokens[*token_pos]->tk_value, "&") == 0 ||
            strcmp(tokens[*token_pos]->tk_value, "-") == 0))
    {
         //Create a Unary Node
        ASTNode *ast_unary = malloc(sizeof(ASTNode ));
        ast_unary->node_type = NODE_UNARY_OP;
        ast_unary->unary_op_node.op = tokens[*token_pos]->tk_value;
        (*token_pos)++;
        ast_unary->unary_op_node.right = parseExpr_Precedence1(tokens, token_pos);
        
        ASTNode *ast_number = tryFoldUnary(ast_unary->unary_op_node.right, ast_unary->unary_op_node.op);

        if (ast_number != NULL)
        {
            operand = ast_number;
            continue;
        }

        operand = ast_unary;
    }

    if (operand != NULL) { return operand; }

    return parseExpr_Precedence1(tokens, token_pos);
}



// Subscription and field access. [] and . and -> 
// Precedence 1.
ASTNode *parseExpr_Precedence1(Token **tokens, int *token_pos)
{
    printf("Parsing expr_precedence 1\n");
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
            ASTNode *ast_subscript = malloc(sizeof(ASTNode));
            ast_subscript->node_type = NODE_SUBSCRIPT;
            ast_subscript->subscript_node.line_number = tokens[*token_pos]->line_number;
            ast_subscript->subscript_node.base = base;

            ast_subscript->subscript_node.index = parseExpression(tokens, token_pos);
            demand_token(tokens, token_pos, TOKEN_SYMBOL, "]");

            base = ast_subscript;
        }
        
        // Field access
        else if (strcmp(tokens[*token_pos]->tk_value, ".") == 0)
        {
            (*token_pos)++;
            ASTNode *ast_field_access = malloc(sizeof(ASTNode));
            ast_field_access->node_type = NODE_FIELD_ACCESS;
            ast_field_access->field_access_node.base = base; 
            ast_field_access->field_access_node.field_name = tokens[*token_pos]->tk_value;
            (*token_pos)++;

            base = ast_field_access;
        }

        // Ptr field access 
        else if (strcmp(tokens[*token_pos]->tk_value, "->") == 0)
        {
            (*token_pos)++;
            ASTNode *ast_ptr_field_access = malloc(sizeof(ASTNode));
            ast_ptr_field_access->node_type = NODE_PTR_FIELD_ACCESS;
            ast_ptr_field_access->ptr_field_access_node.base = base;
            ast_ptr_field_access->ptr_field_access_node.field_name = tokens[*token_pos]->tk_value;
            (*token_pos)++;

            base = ast_ptr_field_access;
        }

        // Post fix operators
        else if (strcmp(tokens[*token_pos]->tk_value, "++") == 0 || strcmp(tokens[*token_pos]->tk_value, "--") == 0)
        {
            ASTNode *ast_postfix_op = malloc(sizeof(ASTNode));
            ast_postfix_op->node_type = NODE_POSTFIX_OP;
            ast_postfix_op->postfix_op_node.left = base;
            ast_postfix_op->postfix_op_node.op = tokens[*token_pos]->tk_value;
            (*token_pos)++;

            base = ast_postfix_op;
        }

    }

    return base;
}




>>>>>>> Stashed changes

ASTNode *tryFoldBinary(ASTNode *left, ASTNode *right, char *op)
{
    if (left->node_type != NODE_NUMBER || right->node_type != NODE_NUMBER) { return NULL; }

    int64_t lnumber = left->number_node.number_value;
    int64_t rnumber = right->number_node.number_value;

    int64_t result = foldBinOperation(lnumber, rnumber, op);

    ASTNode *ast_number = malloc(sizeof(ASTNode));
    ast_number->node_type = NODE_NUMBER;
    ast_number->number_node.number_value = result;

    return ast_number;
}


ASTNode *tryFoldUnary(ASTNode *right, char *op)
{
    if (right->node_type != NODE_NUMBER) { return NULL; }

    int64_t rnumber = right->number_node.number_value;
    int64_t result = foldUnaryOperation(rnumber, op);

    ASTNode *ast_number = malloc(sizeof(ASTNode));
    ast_number->node_type = NODE_NUMBER;
    ast_number->number_node.number_value = result;

    return ast_number;

}

int64_t foldBinOperation(int64_t left, int64_t right, char *binary_op)
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
    if (strcmp(binary_op, "||") == 0) { return left || right; }


    fprintf(stderr, "Unknown binary operation to fold: %s \n", binary_op);
    exit(1);
}

int64_t foldUnaryOperation(int64_t right, char *unary_op)
{
    if (strcmp(unary_op, "-") == 0) { return -1 * right; }
    if (strcmp(unary_op, "!") == 0) { return !right; }

    fprintf(stderr, "Unknown unary operation to fold: %s \n", unary_op);
    exit(1);
}

//Parse single units
ASTNode *parseUnit(Token **tokens, int *token_pos)
{
    //Unary operations
    if (tokens[*token_pos]->tk_type == TOKEN_OPERATOR)
    {
        //Create a Unary Node
        ASTNode *ast_unary = malloc(sizeof(ASTNode ));
        ast_unary->node_type = NODE_UNARY_OP;
        ast_unary->unary_op_node.op = tokens[*token_pos]->tk_value;
        (*token_pos)++;
        ast_unary->unary_op_node.right = parseUnit(tokens, token_pos);
        
        ASTNode *ast_number = tryFoldUnary(ast_unary->unary_op_node.right, ast_unary->unary_op_node.op);
        if (ast_number != NULL)
        {
            free(ast_unary);
            return ast_number;
        }
        

        return ast_unary;
    }
    

    //Function calls:  identifier + "("
    else if (tokens[*token_pos]->tk_type == TOKEN_IDENTIFIER &&
            strcmp(tokens[*token_pos + 1]->tk_value, "(") == 0)
    {
        //Create a Function Call Node
        ASTNode *ast_funccall = malloc(sizeof(ASTNode ));
        ast_funccall->node_type = NODE_FUNC_CALL;
        ast_funccall->funccall_node.identifier = tokens[*token_pos]->tk_value;
        ast_funccall->funccall_node.params_expr = malloc(100 * sizeof(ASTNode *));
        ast_funccall->funccall_node.params_type = malloc(100 * sizeof(char *));
        ast_funccall->funccall_node.params_count = 0;
        //Move past the identifier and "("
        (*token_pos) += 2;
        while (strcmp(tokens[*token_pos]->tk_value, ")") != 0)
        {
            ast_funccall->funccall_node.params_expr[ast_funccall->funccall_node.params_count] = parseExpression(tokens, token_pos);
            ast_funccall->funccall_node.params_count++;
            expect_token(tokens, token_pos,TOKEN_SYMBOL, ",");
        }

        demand_token(tokens, token_pos,TOKEN_SYMBOL, ")");

        return ast_funccall;
    }

    //Char
    else if (tokens[*token_pos]->tk_type == TOKEN_CHAR)
    {
        //Create a char node
        ASTNode *ast_char = malloc(sizeof(ASTNode ));
        ast_char->node_type = NODE_CHAR;
        ast_char->char_node.char_value = tokens[*token_pos]->tk_value[0];
        (*token_pos)++;
        return ast_char;
    }

    //String
    else if (tokens[*token_pos]->tk_type == TOKEN_STR)
    {
        //Create a str node
        ASTNode *ast_str = malloc(sizeof(ASTNode ));
        ast_str->node_type = NODE_STR;
        ast_str->str_node.str_value = tokens[*token_pos]->tk_value;
        (*token_pos)++;
        return ast_str;
    }


    //Numbers
    else if (tokens[*token_pos]->tk_type == TOKEN_NUMBER)
    {
        //Create a Number node
        ASTNode *ast_number = malloc(sizeof(ASTNode ));
        ast_number->node_type = NODE_NUMBER;

        const char *str = tokens[*token_pos]->tk_value;
        char *endptr;
        int64_t n = strtoll(str, &endptr, 10);
        ast_number->number_node.number_value = n;
        //Advance post the number
        (*token_pos)++;

        return ast_number;
    }


    //Booleans
    else if (tokens[*token_pos]->tk_type == TOKEN_KEYWORD && (strcmp(tokens[*token_pos]->tk_value, "true") == 0 ||
            strcmp(tokens[*token_pos]->tk_value, "false") == 0 ))
    {
        //Create a Bool node
        ASTNode *ast_bool = malloc(sizeof(ASTNode ));
        ast_bool->node_type = NODE_BOOL;
        ast_bool->bool_node.bool_value = tokens[*token_pos]->tk_value;
        //Advance post bool value
        (*token_pos)++;

        return ast_bool;
    }

    //Object field (identifier + "." + identifier)
    else if (tokens[*token_pos]->tk_type == TOKEN_IDENTIFIER && strcmp(tokens[*token_pos + 1]->tk_value, ".") == 0
                && tokens[*token_pos + 2]->tk_type == TOKEN_IDENTIFIER)
    {
        //Create instance node
        ASTNode *ast_instance = malloc(sizeof(ASTNode ));
        ast_instance->node_type = NODE_INSTANCE;
        ast_instance->instance_node.instance_identifier = tokens[*token_pos]->tk_value;
        //Advance past identifier
        (*token_pos)++;
        //Advance past "."
        (*token_pos)++;
        ast_instance->instance_node.field_identifier = tokens[*token_pos]->tk_value;
        //Advance past field identifier
        (*token_pos)++;
        return ast_instance;
    }

    //Deref field node
    else if (strcmp(tokens[*token_pos]->tk_value, "(") == 0 && strcmp(tokens[*token_pos + 1]->tk_value, "*") == 0)
    {
        (*token_pos) += 2;

        ASTNode *pointer_expr = parseExpression(tokens, token_pos);
        demand_token(tokens, token_pos, TOKEN_SYMBOL, ")");
        demand_token(tokens, token_pos,TOKEN_SYMBOL, ".");

        ASTNode *ast_deref_field = malloc(sizeof(ASTNode ));
        ast_deref_field->node_type = NODE_DEREF_FIELD;
        ast_deref_field->deref_field_node.pointer_expr = pointer_expr;
        ast_deref_field->deref_field_node.field_name = tokens[*token_pos]->tk_value;
        (*token_pos)++;
        return ast_deref_field;
    }

    //Variables
    else if (tokens[*token_pos]->tk_type == TOKEN_IDENTIFIER)
    {
        //Create an Identifier Node
        ASTNode *ast_id = malloc(sizeof(ASTNode ));
        ast_id->node_type = NODE_IDENTIFIER;
        ast_id->identifier_node.name = tokens[*token_pos]->tk_value;
        //Advance post the identifier
        (*token_pos)++;
        return ast_id;
    }

    else if (strcmp(tokens[*token_pos]->tk_value,"sizeof") == 0)
    {
        //Advance past "sizeof"
        (*token_pos)++;
        //Create a sizeof node
        ASTNode *ast_sizeof = malloc(sizeof(ASTNode));
        ast_sizeof->node_type = NODE_SIZEOF;
        ast_sizeof->sizeof_node.expr = NULL;
        ast_sizeof->sizeof_node.type = NULL;
        demand_token(tokens, token_pos, TOKEN_SYMBOL, "(");
        
        //Built-in type
        if (tokens[*token_pos]->tk_type == TOKEN_TYPE)
        {
            ast_sizeof->sizeof_node.type = tokens[*token_pos]->tk_value;
            (*token_pos)++;
            demand_token(tokens,token_pos, TOKEN_SYMBOL, ")");
            return ast_sizeof;
        }

        //Object identifier or enum identifier 
        if (expect_token(tokens, token_pos, TOKEN_KEYWORD, "object") || expect_token(tokens, token_pos, TOKEN_KEYWORD, "enum"))
        {
            ast_sizeof->sizeof_node.type = tokens[*token_pos]->tk_value;
            (*token_pos)++;
            demand_token(tokens,token_pos, TOKEN_SYMBOL, ")");
            return ast_sizeof;
        }

        //Identifier (Can be variable or ObjectType)
        if (tokens[*token_pos]->tk_type == TOKEN_IDENTIFIER)
        {
            ast_sizeof->sizeof_node.expr = parseExpression(tokens,token_pos);
            demand_token(tokens,token_pos, TOKEN_SYMBOL, ")");
            return ast_sizeof;
        }


    }

    //Type casting. ( type ) Expr
    //(int8), (ptr<int8>), (Node), (object Node)
    else if (strcmp(tokens[*token_pos]->tk_value, "(") == 0 &&
            (tokens[*token_pos + 1]->tk_type == TOKEN_TYPE || 
        tokens[*token_pos + 1]->tk_type == TOKEN_IDENTIFIER || 
     (  (strcmp(tokens[*token_pos + 1]->tk_value, "object") == 0 || strcmp(tokens[*token_pos + 1]->tk_value,"enum") == 0 ) && 
        tokens[*token_pos + 1]->tk_type == TOKEN_IDENTIFIER)) && 
    (strcmp(tokens[*token_pos + 2]->tk_value,")") == 0 || 
      strcmp(tokens[*token_pos + 3]->tk_value, ")") == 0 ||
     strncmp(tokens[*token_pos + 1]->tk_value, "ptr",3) == 0))
    {
        ASTNode *ast_cast = malloc(sizeof(ASTNode));
        ast_cast->node_type = NODE_CAST;
        //Get past "("
        (*token_pos)++;
        
        //Get rid of possible 'object'
        expect_token(tokens, token_pos, TOKEN_KEYWORD, "object");
        expect_token(tokens, token_pos, TOKEN_KEYWORD, "enum");
        //If cast is ptr <type>
        if (strncmp(tokens[*token_pos]->tk_value, "ptr",3) == 0)
        {
            char *type = resolvePtrType(tokens, token_pos);
            ast_cast->cast_node.castType = type;
            demand_token(tokens, token_pos, TOKEN_SYMBOL, ")");
            ast_cast->cast_node.expr = parseExpression(tokens, token_pos);
            return ast_cast;
        }

        //Else type is immediate
        ast_cast->cast_node.castType = tokens[*token_pos]->tk_value;
        (*token_pos)++;
        demand_token(tokens, token_pos, TOKEN_SYMBOL, ")");
        ast_cast->cast_node.expr = parseExpression(tokens, token_pos);
        return ast_cast;
    }




    // Parenthesis
    else if (tokens[*token_pos]->tk_type == TOKEN_SYMBOL &&
            strcmp(tokens[*token_pos]->tk_value, "(") == 0 )
    {
        //Move past the parenthesis
        (*token_pos) ++;
        ASTNode *expr = parseExpression(tokens, token_pos);
        //Remember to get past the closing one

        demand_token(tokens, token_pos,TOKEN_SYMBOL, ")");
        return expr;
    }

<<<<<<< Updated upstream
=======
    // Maybe in the future allow reassingments of arrays by arr = [1,2,3]. ( C does not allow )
    // // Special case. Expression begins with '[' -> array init 
    // else if (strcmp(tokens[*token_pos]->tk_value, "[") == 0)
    // {
    //     (*token_pos)++;
    //     ASTNode *expr = parseArrayInit(tokens, token_pos, );
    //     return ast_assignment;
    // }
    
    // Not supported
>>>>>>> Stashed changes
    else
    {
        fprintf(stderr, "Unit type is not supported: %s (%s) \n",
                tokenTypeToStr(tokens[*token_pos]->tk_type), tokens[*token_pos]->tk_value);
        exit(1);
    }
}



void print_ast(ASTNode *program, int indent)
{
    for (int i = 0; i < indent; i++) printf("  ");

    switch (program->node_type)
    {
        case NODE_ASSIGNMENT:
        {
            printf("Assignment: %s Type: %s \n", program->assignment_node.identifier, program->assignment_node.type);
            print_ast(program->assignment_node.expression, indent + 1);
            break;
        }

        case NODE_REASSIGNMENT:
        {
            printf("Reassignment: %s\n", program->reassignment_node.identifier);
            print_ast(program->reassignment_node.expression, indent + 1);
            break;
        }
        
<<<<<<< Updated upstream
=======
        case NODE_SUBSCRIPT:
        {
            printf("Subscript: \n");
            for (int i = 0; i < indent; i++) printf("  ");
            printf("Base Array:\n");
            print_ast(program->subscript_node.base, indent + 1);

            for (int i = 0; i < indent; i++) printf("  ");
            printf("Index: \n");
            print_ast(program->subscript_node.index, indent + 1);
            break;
        }
        
        case NODE_ARRAY_INIT:
        {
            printf("Array Init. Identifier = %s\n", program->array_init_node.arr_name);
            for (int k = 0; k < program->array_init_node.size; k++)
            {
                for (int i = 0; i < indent; i++) printf("  ");
                printf("Element %i = \n", k);
                print_ast(program->array_init_node.elements[k], indent + 1);
            }
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
        
>>>>>>> Stashed changes
        case NODE_CB_ASSIGNMENT:
        {
                printf("Callback assignment. Alias: %s, %s(", program->cbassignment_node.cb_identifier,
                                                            program->cbassignment_node.fn_identifider);
                for (int k = 0; k < program->cbassignment_node.param_count; k++)
                {
                    printf("%s , ", program->cbassignment_node.param_types[k]);
                }
                printf(")\n");
                break;
            }
        case NODE_PTR_REASSIGNMENT:
        {
            printf("Ptr reassignment. Deref level: %i. Lvalue: \n", program->ptr_reassignment_node.deref_level);
            print_ast(program->ptr_reassignment_node.lvalue, indent + 1);
            print_ast(program->ptr_reassignment_node.expression, indent + 1);
            break;
        }

        case NODE_PTR_FIELDREASSIGNMENT:
        {
            printf("Ptr Field Reassignment. Ptr identifier: %s, Field: %s  \n", program->ptr_fieldreassign_node.ptr_identifier,
                   program->ptr_fieldreassign_node.field_identifier);
            print_ast(program->ptr_fieldreassign_node.expression, indent + 1);
            break;
        }

        case NODE_DEREF_FIELD:
        {
            printf("Ptr Deref Field. Field: %s \n", program->deref_field_node.field_name);
            print_ast(program->deref_field_node.pointer_expr, indent + 1);
            break;
        }

        case NODE_INSTANCE_REASSIGNMENT:
        {
            printf("Instance Reassignment: %s.%s\n", program->field_reassign_node.instance_identifier, program->field_reassign_node.field_identifier);
            print_ast(program->field_reassign_node.expression, indent + 1);
            break;
        }

        case NODE_DECLARATION:
        {
            printf("Declaration: %s Type: %s \n", program->declaration_node.identifier, program->declaration_node.type);
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

        case NODE_STDALONE_POSTFIX_OP:
        {
            printf("Stdalone Postfix op: %s \n", program->stdalone_postfix_op_node.op);
            print_ast(program->stdalone_postfix_op_node.left, indent + 1);
            break;
        }

        case NODE_CAST:
        {
            printf("Cast to: %s \n", program->cast_node.castType);
            print_ast(program->cast_node.expr, indent + 1);
            break;
        }

        case NODE_SIZEOF:
        {
            printf("Sizeof : ");
            if (program->sizeof_node.type != NULL)
            {
                printf("%s\n", program->sizeof_node.type);
            }
            else if (program->sizeof_node.expr != NULL)
            {
                printf("\n");
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

        case NODE_SYSCALL:
        {
            printf("Syscall. OPCODE: %s, Operand: \n", program->syscall_node.syscall_identifier);
            print_ast(program->syscall_node.operand, indent + 1);
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
            print_ast(program->if_node.condition_expr, indent + 1);
            if (program->if_node.else_body != NULL)
            {
                for (int i = 0; i < indent; i++) printf("  ");
                printf("Else: \n");
                print_ast(program->if_node.else_body, indent + 1);
            }
            break;
        }

        case NODE_SKIP:
        {
            printf("skip \n");
            break;
        }

        case NODE_STOP:
        {
            printf("stop \n");
            break;
        }

        case NODE_FOR:
        {
            printf("For: \n");
            for (int i = 0; i < indent; i++) printf("  "); printf("Assignment: \n");
            print_ast(program->for_node.assignment_expr, indent + 1);
            for (int i = 0; i < indent; i++) printf("  "); printf("Condition: \n");
            print_ast(program->for_node.condition_expr, indent + 1);
            for (int i = 0; i < indent; i++) printf("  "); printf("Update: \n");
            print_ast(program->for_node.reassignment_expr, indent + 1);
            print_ast(program->for_node.body, indent + 1);
            break;
        }

        case NODE_FOREACH:
        {
            printf("Foreach: \n");
            print_ast(program->foreach_node.foreach_declaration, indent + 1);
            for (int i = 0; i < indent; i++) printf("  ");
            printf("in: \n");
            print_ast(program->foreach_node.ptr_array_expr, indent + 1);
            for (int i = 0; i < indent; i++) printf("  ");
            printf("within: \n");
            for (int i = 0; i < indent; i++) printf("  ");
            printf("Lower limit: \n");
            print_ast(program->foreach_node.lower_limit_expr, indent + 1);

            for (int i = 0; i < indent; i++) printf("  ");
            printf("Upper limit: \n");
            print_ast(program->foreach_node.upper_limit_expr, indent + 1);
            print_ast(program->foreach_node.body, indent + 1);
            break;
        }

        case NODE_FUNC_DEF:
        {
            printf("Func (rt : %s ) def: %s.\n", program->funcdef_node.return_type, program->funcdef_node.func_name);
            for (int j = 0; j < program->funcdef_node.params_count; j++)
            {
                for (int k = 0; k < indent + 1; k++) printf("  ");
                printf("Param %i: ",j);
                switch (program->funcdef_node.params[j]->supertype) 
                {       
                    case PARAM_IMMEDIATE:
                    {
                        printf("%s %s \n",program->funcdef_node.params[j]->immediateParam.type,
                               program->funcdef_node.params[j]->immediateParam.param_name);
                        break;
                    }

                    case PARAM_FUNCTION:
                    {
                        printf("%s %s ( ", program->funcdef_node
                               .params[j]->functionParam.type,program->funcdef_node.params[j]->functionParam.identifier);
                        for (int l = 0; l < program->funcdef_node.params[j]->functionParam.params_count; l++)
                        {
                            printf("%s, ", program->funcdef_node.params[j]->functionParam.params_type[l]);
                        }
                        printf(") => %s\n", program->funcdef_node.params[j]->functionParam.return_type);
                        break;
                    }
                }
            }
            print_ast(program->funcdef_node.body, indent + 1);
            break;
        }

        case NODE_EXTERN_FUNC_DEF:
        {
            printf("Extern Func (rt : %s ) def: %s.\n", program->extern_func_def_node.return_type, program->extern_func_def_node.func_name);
            for (int j = 0; j < program->extern_func_def_node.params_count; j++)
            {
                for (int k = 0; k < indent + 1; k++) printf("  ");
                printf("Param %i: ",j);
                switch (program->extern_func_def_node.params[j]->supertype) 
                {       
                    case PARAM_IMMEDIATE:
                    {
                        printf("%s %s \n",program->extern_func_def_node.params[j]->immediateParam.type,
                               program->extern_func_def_node.params[j]->immediateParam.param_name);
                        break;
                    }

                    case PARAM_FUNCTION:
                    {
                        printf("%s %s ( ", program->extern_func_def_node
                               .params[j]->functionParam.type,program->extern_func_def_node.params[j]->functionParam.identifier);
                        for (int l = 0; l < program->extern_func_def_node.params[j]->functionParam.params_count; l++)
                        {
                            printf("%s, ", program->extern_func_def_node.params[j]->functionParam.params_type[l]);
                        }
                        printf(") => %s\n", program->extern_func_def_node.params[j]->functionParam.return_type);
                        break;
                    }
                }
            }
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

        case NODE_STDALONE_FUNC_CALL:
        {
            printf("StdAlone call %s. Params: \n", program->stdalone_func_call_node.identifier);
            for (int j = 0; j < program->stdalone_func_call_node.params_count; j++)
            {
                print_ast(program->stdalone_func_call_node.params_expr[j], indent + 1);
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
            if (program->object_node.parent != NULL)
            {
                printf("inherits %s ", program->object_node.parent);
            }
            printf("\n");
            for (int j = 0; j < program->object_node.declaration_count; j++)
            {
                print_ast(program->object_node.declarations[j], indent + 1);
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

        case NODE_INSTANCE:
        {
            printf("Instance name: %s, field name: %s \n", program->instance_node.instance_identifier, program->instance_node.field_identifier);
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
            printf("Unknown node type in print_ast()\n");
            break;
        }


    }

}


void free_ast(ASTNode *node) {
    if (node == NULL)
    {
        return;
    }
    //printf("Freeing up node with type: %i \n", node->node_type);

    switch (node->node_type)
    {
        case NODE_BLOCK:
        {
            for (int i = 0; i < node->block_node.statement_count; i++)
            {
                free_ast(node->block_node.statements[i]);
            }
            free(node->block_node.statements);
            break;
        }

        case NODE_ASSIGNMENT: {
            free_ast(node->assignment_node.expression);
            break;
        }

        case NODE_REASSIGNMENT: {
            free_ast(node->reassignment_node.expression);
            break;
        }

        case NODE_PTR_REASSIGNMENT: {
            free_ast(node->ptr_reassignment_node.lvalue);
            free_ast(node->ptr_reassignment_node.expression);
            break;
        }

        case NODE_PTR_FIELDREASSIGNMENT: {
            free_ast(node->ptr_fieldreassign_node.expression);
            break;
        }

        case NODE_DEREF_FIELD: {
            free_ast(node->deref_field_node.pointer_expr);
            break;
        }

        case NODE_INSTANCE_REASSIGNMENT: {
            free_ast(node->field_reassign_node.expression);
            break;
        }
        case NODE_DECLARATION: {
            // No additional memory to free here
            break;
        }

        case NODE_UNARY_OP: {
            free_ast(node->unary_op_node.right);
            break;
        }

        case NODE_BINARY_OP: {
            free_ast(node->binary_op_node.left);
            free_ast(node->binary_op_node.right);
            break;
        }

        case NODE_SYSCALL: {
            free_ast(node->syscall_node.operand);
            break;
        }

        case NODE_NULL: {
            // No additional memory to free here
            break;
        }

        case NODE_NUMBER: {
            // No additional memory to free here
            break;
        }

        case NODE_CHAR: {
            // No additional memory to free here
            break;
        }


        case NODE_STR: {
            //free(node->str_node.str_value);
            break;
        }

        case NODE_BOOL: {
            // No additional memory to free here
            break;
        }

        case NODE_IDENTIFIER: {
            free(node->identifier_node.name);
            break;
        }

        case NODE_WHILE: {
            free_ast(node->while_node.condition_expr);
            free_ast(node->while_node.body);
            break;
        }

        case NODE_IF: {
            free_ast(node->if_node.condition_expr);
            free_ast(node->if_node.body);
            if (node->if_node.else_body != NULL) {
                free_ast(node->if_node.else_body);
            }
            break;
        }

        case NODE_FOR: {
            free_ast(node->for_node.assignment_expr);
            free_ast(node->for_node.condition_expr);
            free_ast(node->for_node.reassignment_expr);
            free_ast(node->for_node.body);
            break;
        }

        case NODE_FUNC_DEF: 
        {
            /*
            for (int i = 0; i < node->funcdef_node.params_count; i++) {
                free(node->funcdef_node.params[i]);
                free(node->funcdef_node.param[i]);
            }
            free(node->funcdef_node.params);
            free(node->funcdef_node.params_type);
            free_ast(node->funcdef_node.body);
            */
            break;
        }

        case NODE_FUNC_CALL: {
            for (int i = 0; i < node->funccall_node.params_count; i++) {
                free_ast(node->funccall_node.params_expr[i]);
            }
            free(node->funccall_node.params_expr);
            //free(node->funccall_node.params_type);
            break;
        }

        case NODE_RETURN: {
            if (node->return_node.return_expr != NULL) {
                free_ast(node->return_node.return_expr);
            }
            break;
        }

        case NODE_OBJECT: 
        {
            for (int i = 0; i < node->object_node.declaration_count; i++) {
                free_ast(node->object_node.declarations[i]);
            }
            free(node->object_node.declarations);
            break;
        }

        case NODE_INSTANCE:
        {
            // No additional memory to free here
            break;
        }

        default:
        {
            //fprintf(stderr, "Unknown node type in free_ast()\n");
            break;
        }
    }

    // Finally, free the node itself
    free(node);
}


<<<<<<< Updated upstream
=======
int is_statement(ASTNode *node)
{
    switch(node->node_type)
    {
        case NODE_ASSIGNMENT:
        case NODE_RETURN:
        case NODE_DECLARATION:
        case NODE_WHILE:
        case NODE_FOR:
        case NODE_FOREACH:
        case NODE_IF:
        case NODE_STDALONE_FUNC_CALL:
        case NODE_STDALONE_POSTFIX_OP:
        case NODE_REASSIGNMENT: return 1;
        
        default: return 0;
    }
}

int is_pushing_scope(ASTNode *node)
{
    switch (node->node_type) 
    {
        case NODE_FUNC_DEF:
        case NODE_IF:
        case NODE_FOR:
        case NODE_FOREACH:
        case NODE_WHILE: return 1;

        default: return 0;
    }
}
>>>>>>> Stashed changes

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
        case NODE_CB_ASSIGNMENT: return "CB_ASSIGNMENT";
        case NODE_INSTANCE_REASSIGNMENT: return "INSTANCE_REASSIGNMENT";
        case NODE_PTR_FIELDREASSIGNMENT: return "PTR_FIELD_REASSIGNMENT";
        case NODE_EXTERN_FUNC_DEF: return "EXTERN_FUNC_DEF";
        case NODE_DEREF_FIELD: return "DEREF_FIELD";
        case NODE_PTR_REASSIGNMENT: return "PTR_REASSIGNMENT";
        case NODE_DECLARATION: return "DECLARATION";
        case NODE_NULL: return "NULL";
        case NODE_ENUM: return "ENUM";
        case NODE_FOREACH: return "FOREACH";
        case NODE_SIZEOF: return "SIZEOF";
        case NODE_SKIP: return "SKIP";
        case NODE_STDALONE_FUNC_CALL: return "STDALONE_FUNC_CALL";
        case NODE_FUNC_CALL: return "FUNC_CALL";
        case NODE_FUNC_DEF: return "FUNC_DEF";
        case NODE_STOP: return "STOP";
        case NODE_USE_DIRECTIVE: return "USE_DIRECTIVE";
        case NODE_UNARY_OP: return "UNARY_OP";
        case NODE_BOOL: return "BOOL";
        case NODE_IDENTIFIER: return "IDENTIFIER";
        case NODE_CAST: return "CAST";
        case NODE_CHAR: return "CHAR";
        case NODE_FOR: return "FOR";
        case NODE_IF: return "IF";
        case NODE_INSTANCE: return "INSTANCE";
        case NODE_OBJECT: return "OBJECT";
        case NODE_STR: return "STRING";
        case NODE_WHILE: return "WHILE";
        case NODE_ARRAY_INIT: return "ARRAY_INIT";
        case NODE_FIELD_ACCESS: return "FIELD_ACCESS";
        case NODE_PTR_FIELD_ACCESS: return "PTR_FIELD_ACCESS";
        case NODE_POSTFIX_OP: return "POSTFIX_OP";
        case NODE_STDALONE_POSTFIX_OP: return "STDALONE_POSTFIX_OP";

        default: return "UNKNOWN AST TYPE";
    }
}
