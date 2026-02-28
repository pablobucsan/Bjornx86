//
// Created by pablo on 28/02/2025.
//

#include "../include/analyzer.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <stdint.h>
#include <inttypes.h>

SymbolTable *gb_symbolTable = NULL;
FunctionTable *gb_functionTable = NULL;
ObjectTable *gb_objectTable = NULL;
ClassTable *gb_classTable = NULL;
EnumTable *gb_enumTable = NULL;
StringTable *gb_stringTable = NULL;



Context context = {
    .current_scope = 0,
    .stack_index = 0,
    .if_block_id = 0,
    .enum_block_id = 0,
    .object_block_id = 0,
    .union_block_id = 0,
    .class_block_id = 0,
    .elseif_block_id = 0,
    .else_block_id = 0,
    .for_block_id = 0,
    .foreach_block_id = 0,
    .while_block_id = 0,
    .ext_funct_id = 0,
    .function_block_id = 0,
    .skip_right_id = 0,
    .current_class = NULL,
    .current_object = NULL,
};


LabelContext label_context;

void update_tracker(ASTNode *node)
{
    switch (node->node_type)
    {
        case NODE_USE_DIRECTIVE:
        {
            // Handled individually
            //tracker.current_src_file = node->use_node.filepath;
            break;
        }
        case NODE_ARRAY_INIT:
        {
            tracker.current_line = node->array_init_node.line_number; 
            break;
        }

        case NODE_UNION:
        {
            tracker.current_line = node->union_node.line_number;
            break;
        }

        case NODE_ASSIGNMENT:
        {
            tracker.current_line = node->assignment_node.line_number;
            break;
        }
        case NODE_BINARY_OP:
        {
            tracker.current_line = node->binary_op_node.line_number;
            break;
        }
        case NODE_BLOCK:
        {
            tracker.current_line = node->block_node.line_number;
            break;
        }
        case NODE_BOOL:
        {
            tracker.current_line = node->bool_node.line_number;
            break;
        }

        case NODE_CAST:
        {
            tracker.current_line = node->cast_node.line_number;
            break;
        }
        case NODE_CHAR:
        {
            tracker.current_line = node->char_node.line_number;
            break;
        }
        case NODE_DECLARATION:
        {
            tracker.current_line = node->declaration_node.line_number;
            break;
        }
        case NODE_ELSEIF:
        {
            tracker.current_line = node->elseif_node.line_number;
            break;
        }

        case NODE_ENUM:
        {
            tracker.current_line = node->enum_node.line_number;
            break;
        }
        case NODE_EXTERN_FUNC_DEF:
        {
            tracker.current_line = node->extern_func_def_node.line_number;
            break;
        }
        case NODE_FIELD_ACCESS:
        {
            tracker.current_line = node->field_access_node.line_number;
            break;
        }

        case NODE_FOR:
        {
            tracker.current_line = node->for_node.line_number;
            break;
        }

        case NODE_FOREACH:
        {
            tracker.current_line = node->foreach_node.line_number;
            break;
        }

        case NODE_FUNC_CALL:
        {
            tracker.current_line = node->funccall_node.line_number;
            break;
        }
        case NODE_FUNC_DEF:
        {
            tracker.current_line = node->funcdef_node.line_number;
            break;
        }

        case NODE_IDENTIFIER:
        {
            tracker.current_line = node->identifier_node.line_number;
            break;
        }

        case NODE_IF:
        {
            tracker.current_line = node->if_node.line_number;
            break;
        }

        case NODE_NUMBER:
        {
            tracker.current_line = node->number_node.line_number;
            break;
        }

        case NODE_OBJECT:
        {
            tracker.current_line = node->object_node.line_number;
            break;
        }

        case NODE_POSTFIX_OP:
        {
            tracker.current_line = node->postfix_op_node.line_number;
            break;
        }

        case NODE_PTR_FIELD_ACCESS:
        {
            tracker.current_line = node->ptr_field_access_node.line_number;
            break;
        }

        case NODE_REASSIGNMENT:
        {
            tracker.current_line = node->reassignment_node.line_number;
            break;
        }

        case NODE_RETURN:
        {
            tracker.current_line = node->return_node.line_number;
            break;
        }


        case NODE_STR:
        {
            tracker.current_line = node->str_node.line_number;
            break;
        }

        case NODE_SUBSCRIPT:
        {
            tracker.current_line = node->subscript_node.line_number;
            break;
        }

        case NODE_UNARY_OP:
        {
            tracker.current_line = node->unary_op_node.line_number;
            break;
        }

        case NODE_WHILE:
        {
            tracker.current_line = node->while_node.line_number;
            break;
        }

        default:
        {
            break;
        }
    }
}


/**
 * ===============================================================================
 * ANALYSIS CONTEXT    
 * ===============================================================================   
 */

void initAnalysisContext()
{
    label_context.capacity = INIT_LABEL_CTX_CAPACITY;
    label_context.count = 0;
    label_context.index = 0;
    label_context.labels = malloc(sizeof(Label *) * label_context.capacity);
}


void pushCtxLabel(Label *l)
{
    if (label_context.count + 1 >= label_context.capacity){
        label_context.capacity *= 2;
        label_context.labels = realloc(label_context.labels, sizeof(Label *) * label_context.capacity);
    }

    label_context.labels[label_context.count++] = l;
}

void popCtxLabel()
{
    label_context.count--;
}

Label *getCurrentCtxLabel()
{
    if (label_context.count == 0){
        return NULL;
    }

    return label_context.labels[label_context.count - 1];
}


char * ctxToString(ContextType ctx_type)
{
    switch (ctx_type) 
    {
        case CTX_WHILE: return "WHILE";
        case CTX_FUNCTION: return "FUNCTION";
        case CTX_FOR: return "FOR";
        case CTX_CLASS: return "CLASS";
        case CTX_FOREACH: return "FOREACH";
        case CTX_IF: return "IF";
        case CTX_ELSE: return "ELSE";
        case CTX_UNION: return "UNION";
        case CTX_NONE: return "NONE";
        case CTX_ENUM: return "ENUM";
        case CTX_OBJECT: return "OBJECT";
        case CTX_ELSEIF: return "ELSEIF";
    }

    fprintf(stderr, "Unknown context to turn to string: %i\n", ctx_type);
    exit(1);
}

ContextBlock getCurrentContext(){
    //return context.ctx_type_stack[context.stack_index - 1];
    return context.ctx_block_stack[context.current_scope];
}

ContextType getCurrentContextType(){
    return context.ctx_block_stack[context.current_scope].ctx_type;
}

int getCurrentScope(){
    return context.current_scope;
}

int getCurrentContextBlockId(ContextType contextType){
    switch (contextType) 
    {
        case CTX_NONE: return 0;
        case CTX_FUNCTION: return context.function_block_id;
        case CTX_FOR: return context.for_block_id;
        case CTX_FOREACH: return context.foreach_block_id;
        case CTX_WHILE: return context.while_block_id;
        case CTX_IF: return context.if_block_id;
        case CTX_UNION: return context.union_block_id;
        case CTX_ELSE: return context.else_block_id; 
        case CTX_ELSEIF: return context.elseif_block_id;
        case CTX_OBJECT: return context.object_block_id;
        case CTX_CLASS: return context.class_block_id;
        case CTX_ENUM: return context.enum_block_id;     
        default:{
            fprintf(stderr, "Unknown context type to get the id of: %s\n", ctxToString(contextType));
            exit(1);
        }                
    }
}

Class *getCurrentClass(){
    return context.current_class;
}

Object *getCurrentObject(){
    return context.current_object;
}


void initGlobalTables()
{
    __initGlobalSymbolTable();
    __initGlobalFunctionTable();
    __initGlobalEnumTable();
    __initGlobalObjectTable();
    __initGlobalClassTable();
    __initGlobalStringTable();
}



void incrementContextBlockID(ContextType ctx_type)
{
    switch (ctx_type) 
    {
        case CTX_IF: context.if_block_id++; return;
        case CTX_CLASS: context.class_block_id++; return;
        case CTX_ELSEIF: context.elseif_block_id++; return;
        case CTX_ELSE: context.else_block_id++; return;
        case CTX_FOR: context.for_block_id++; return;
        case CTX_FOREACH: context.foreach_block_id++; return;
        case CTX_WHILE: context.while_block_id++; return;
        case CTX_ENUM: context.enum_block_id++; return;
        case CTX_OBJECT: context.object_block_id++; return;
        case CTX_FUNCTION: context.function_block_id++; return;
        case CTX_UNION: context.union_block_id++; return;
        case CTX_NONE:
        {
            fprintf(stderr, "Cannot increment NONE_block_id as there is only one global scope. \n");
            exit(1);
        }
    }
}


int isAllowedContext(ContextType ctx_type)
{
    ContextType current_ctx = getCurrentContextType();

    // If the current context is NONE, enum defs, object defs and function defs are allowed
    // as well as assignments and such, nothing else
    if (current_ctx == CTX_NONE)
    {
        switch(ctx_type)
        {
            case CTX_ENUM:
            case CTX_OBJECT:
            case CTX_CLASS:
            case CTX_FUNCTION: return 1;

            default: return 0;
        }
    }

    // If the current context is a function, object def and enum def is not allowed, everything else is
    if (current_ctx == CTX_FUNCTION)
    {
        switch(ctx_type)
        {
            case CTX_OBJECT:
            case CTX_CLASS:
            case CTX_ENUM: return 0;

            default: return 1;
        }
    }

    // If the current context is a class, only functions are allowed (first level depth)
    if (current_ctx == CTX_CLASS)
    {
        switch (ctx_type)
        {
            case CTX_FUNCTION: return 1;
            case CTX_UNION: return 1;
            default: return 0;
        }
    }

    // No context is allowed within an enum or object def
    if (current_ctx == CTX_ENUM || current_ctx == CTX_OBJECT)
    {
        switch(ctx_type)
        {
            case CTX_UNION: return 1;
            default: return 0;
        }
    }

    // No func def, object def, enum def or class def inside a loop or ifs
    switch (current_ctx)
    {
        case CTX_FOR:
        case CTX_FOREACH:
        case CTX_WHILE:
        case CTX_IF:
        case CTX_ELSEIF:
        case CTX_ELSE:
        {
            if (ctx_type == CTX_ENUM || ctx_type == CTX_OBJECT || ctx_type == CTX_FUNCTION || ctx_type == CTX_CLASS || ctx_type == CTX_UNION)
            {
                return 0;
            }
        }

        default: return 1;
    }

    // If we get here, is fine
    return 1;
}

int contextPushesScope(ContextType current_ctx)
{

    switch(current_ctx)
    {
        case CTX_FUNCTION:
        case CTX_FOR:
        case CTX_WHILE:
        case CTX_FOREACH:
        case CTX_IF:
        case CTX_CLASS:
        case CTX_OBJECT:                // objects too? 13/12
        case CTX_ELSEIF:
        case CTX_UNION:
        case CTX_ELSE:
        case CTX_ENUM:{
            return 1;
        }

        default: return 0;
    }
}

void pushContext(ContextType ctx_type)
{
    if (context.current_scope + 1 >= MAX_NESTING){
        Error *e = invoke(NESTING_OVERFLOW);
        appendMessageToError(e, fetchErrorMsgBasedOnErrorID(e->error_id), ctxToString(ctx_type), MAX_NESTING);
        addError(e);
    }

    if (!isAllowedContext(ctx_type)){
        Error *e = invoke(CONTEXT_NOT_ALLOWED);
        appendMessageToError(e, fetchErrorMsgBasedOnErrorID(e->error_id), ctxToString(ctx_type), ctxToString(getCurrentContextType()));
        addError(e);
    }
    

    // Increment the scope if applicable
    if (contextPushesScope(ctx_type)){
        ContextBlock cb;
        cb.ctx_type = ctx_type;
        cb.block_id = getCurrentContextBlockId(ctx_type);
        context.current_scope++;
        context.ctx_block_stack[context.current_scope] = cb;
    }

    // Increment the block id and put the context on its right stack position
    incrementContextBlockID(ctx_type);

    //printf("Current scope after pushing: %i\n", context.current_scope);

    //printf("(SCOPE, INDEX) IS NOW AFTER PUSHING: (%i,%i) \n", context.current_scope, context.stack_index);
}

void popContext()
{
    if (context.current_scope < 0){
        fprintf(stderr, "Can't pop context if context is CTX_NONE. \n");
        exit(1);
    }

    ContextType ctx_type = getCurrentContextType();

    if (contextPushesScope(ctx_type)){
        context.current_scope--;
    };

    //context.stack_index--;
    //printf("(SCOPE, INDEX) IS NOW AFTER POPPING: (%i,%i) \n", context.current_scope, context.stack_index);
    //printf("Current scope after popping: %i\n", context.current_scope);
}



/**
 * ===============================================================================
 * END ANALYSIS CONTEXT    
 * ===============================================================================   
 */




/**
 * ===============================================================================
 * LOWERERS     
 * ===============================================================================   
 */

Tag *cloneTag(Tag *t)
{
    Tag *clone = __allocateTag();
    clone->tag_name = mystrdup(t->tag_name);

    return clone;
}

Label *cloneLabel(Label *l)
{
    if (l == NULL){
        return NULL;
    }

    Label *clone;
    // printf("Cloning label.\n");
    // printf("with kind = %i\n", l->kind);

    switch(l->kind)
    {
        case COMPOUND:
        {
            clone = makeCompoundLabel();
            clone->compound_label.start_tag = cloneTag(l->compound_label.start_tag);
            clone->compound_label.intermediate_tags_count = l->compound_label.intermediate_tags_count;
            clone->compound_label.intermediate_tags_capacity = l->compound_label.intermediate_tags_capacity;
            for (int i = 0; i < l->compound_label.intermediate_tags_count; i++){
                clone->compound_label.intermediate_tags[i] = cloneTag(l->compound_label.intermediate_tags[i]);
            }
            clone->compound_label.end_tag = cloneTag(l->compound_label.end_tag);
            break;
        }

        case TAG:
        {
            clone = makeTagLabel(l->tag.tag_name);
            break;
        }
    }

    clone->kind = l->kind;
    return clone;
    
}

/**
 * Can't use strdup() for some things because analyze() populates as it goes some fields,
 * and cloneAST() may be called before those fields are populated, therefore we try to do strdup(NULL) --> crashes
 * Change strdup() for mystrdup()
 */
ASTNode *cloneAST(ASTNode *original)
{
    if (original == NULL || original->node_type == NODE_NULL){
        fprintf(stderr, "Can't clone a NULL AST node\n");
        exit(1);
    }

    ASTNode *clone = calloc(1,sizeof(ASTNode));
    clone->node_type = original->node_type;
    clone->is_stmt = original->is_stmt;
    //printf("Cloning AST with type: %s\n", astTypeToStr(original));
    switch (original->node_type)
    {
        case NODE_BLOCK:
        {
            clone->block_node.line_number = original->block_node.line_number;
            clone->block_node.capacity = original->block_node.capacity;
            clone->block_node.statement_count = original->block_node.statement_count;
            clone->block_node.statements = malloc(sizeof(ASTNode *) * clone->block_node.statement_count);

            for (int i = 0; i < original->block_node.statement_count; i++)
            {
                clone->block_node.statements[i] = cloneAST(original->block_node.statements[i]);
            }

            break;
        }

        case NODE_ASSIGNMENT:
        {
            clone->assignment_node.line_number = original->assignment_node.line_number;
            clone->assignment_node.identifier = mystrdup(original->assignment_node.identifier);
            clone->assignment_node.line_number = original->assignment_node.line_number;
            clone->assignment_node.type = original->assignment_node.type;
            clone->assignment_node.expression = cloneAST(original->assignment_node.expression);

            break;
        }

        case NODE_REASSIGNMENT:
        {
            clone->reassignment_node.expression = cloneAST(original->reassignment_node.expression);
            clone->reassignment_node.line_number = original->reassignment_node.line_number;
            clone->reassignment_node.lvalue = cloneAST(original->reassignment_node.lvalue);
            clone->reassignment_node.op = original->reassignment_node.op;
            clone->reassignment_node.type = original->reassignment_node.type;
            break;
        }
        
        case NODE_SUBSCRIPT:
        {
            clone->subscript_node.base = cloneAST(original->subscript_node.base);
            clone->subscript_node.base_identifier = mystrdup(original->subscript_node.base_identifier);
            clone->subscript_node.base_type = original->subscript_node.base_type;
            clone->subscript_node.element_size = original->subscript_node.element_size;
            clone->subscript_node.index = cloneAST(original->subscript_node.index);
            clone->subscript_node.index_size = original->subscript_node.index_size;
            clone->subscript_node.line_number = original->subscript_node.line_number;
            break;
        }
        
        case NODE_ARRAY_INIT:
        {
            clone->array_init_node.line_number = original->array_init_node.line_number;
            clone->array_init_node.arr_name = mystrdup(original->array_init_node.arr_name);
            clone->array_init_node.capacity = original->array_init_node.capacity;
            clone->array_init_node.elements = malloc(sizeof(ASTNode *) * clone->array_init_node.capacity);

            for (int i = 0; i < clone->array_init_node.capacity; i++)
            {
                clone->array_init_node.elements[i] = cloneAST(original->array_init_node.elements[i]);
            }

            clone->array_init_node.capacity = original->array_init_node.capacity;
            break;
        }

        case NODE_PTR_FIELD_ACCESS:
        {
            clone->ptr_field_access_node.line_number = original->ptr_field_access_node.line_number;
            clone->ptr_field_access_node.base = cloneAST(original->ptr_field_access_node.base);
            clone->ptr_field_access_node.field_name = mystrdup(original->ptr_field_access_node.field_name);
            clone->ptr_field_access_node.type = original->ptr_field_access_node.type;

            break;
        }

        case NODE_FIELD_ACCESS:
        {

            clone->field_access_node.line_number = original->field_access_node.line_number;
            clone->field_access_node.base = cloneAST(original->field_access_node.base);
            clone->field_access_node.field_name = mystrdup(original->field_access_node.field_name);

            clone->field_access_node.type = original->field_access_node.type;
            break;
        }
        

        case NODE_DECLARATION:
        {
            clone->declaration_node.line_number = original->declaration_node.line_number;
            clone->declaration_node.identifier = mystrdup(original->declaration_node.identifier);
            clone->declaration_node.type = original->declaration_node.type;
            break;
        }

        case NODE_UNARY_OP:
        {
            clone->unary_op_node.line_number = original->unary_op_node.line_number;
            clone->unary_op_node.op = mystrdup(original->unary_op_node.op);
            clone->unary_op_node.right = cloneAST(original->unary_op_node.right);
            clone->unary_op_node.size_of_operand = original->unary_op_node.size_of_operand;
            break;
        }

        case NODE_POSTFIX_OP:
        {
            clone->postfix_op_node.line_number = original->postfix_op_node.line_number;
            clone->postfix_op_node.left = cloneAST(original->postfix_op_node.left);
            clone->postfix_op_node.op = mystrdup(original->postfix_op_node.op);
            clone->postfix_op_node.size_of_operand = original->postfix_op_node.size_of_operand;
            clone->postfix_op_node.type = original->postfix_op_node.type;
            break;
        }

        case NODE_CAST:
        {
            clone->cast_node.line_number = original->cast_node.line_number;
            clone->cast_node.castType = original->cast_node.castType;
            clone->cast_node.expr = cloneAST(original->cast_node.expr);
            break;
        }
        case NODE_CONTINUE:
        {
            // printf("About to clone continue...\n");
            clone->continue_node.line_number = original->continue_node.line_number;
            clone->continue_node.tag = NULL;
            if (original->continue_node.tag != NULL){
                clone->continue_node.tag = cloneTag(original->continue_node.tag);
            }
            // printf("Cloned continue\n");
            break;
        }
        case NODE_BREAK:
        {

            clone->break_node.line_number = original->break_node.line_number;
            clone->break_node.tag = NULL;
            if (original->break_node.tag != NULL){
                clone->break_node.tag = cloneTag(original->break_node.tag);
            }
            break;
        }

        case NODE_BINARY_OP:
        {
            //printf("Cloning\n");
            clone->binary_op_node.line_number = original->binary_op_node.line_number;
            clone->binary_op_node.left = cloneAST(original->binary_op_node.left);

            clone->binary_op_node.op = mystrdup(original->binary_op_node.op);

            clone->binary_op_node.right = cloneAST(original->binary_op_node.right);

            break;
        }

        case NODE_NUMBER:
        {
            clone->number_node.line_number = original->number_node.line_number;
            clone->number_node.number_type = original->number_node.number_type;
            clone->number_node.number_value = original->number_node.number_value;
            clone->number_node.representation = original->number_node.representation;
            break;
        }

        case NODE_CHAR:
        {
            clone->char_node.line_number = original->char_node.line_number;
            clone->char_node.char_value = original->char_node.char_value;
            clone->char_node.char_type = original->char_node.char_type;
            break;
        }

        case NODE_STR:
        {
            clone->str_node.line_number = original->str_node.line_number;
            clone->str_node.label = cloneLabel(original->str_node.label);

            clone->str_node.str_value = mystrdup(original->str_node.str_value);
            clone->str_node.str_type = original->str_node.str_type;
            break;
        }

        case NODE_BOOL:
        {
            clone->bool_node.line_number = original->bool_node.line_number;
            clone->bool_node.bool_value = mystrdup(original->bool_node.bool_value);
            clone->bool_node.bool_type = original->bool_node.bool_type;
            break;
        }

        case NODE_IDENTIFIER:
        {
            clone->identifier_node.line_number = original->identifier_node.line_number;
            clone->identifier_node.name = mystrdup(original->identifier_node.name);
            break;
        }

        case NODE_WHILE:
        {
            clone->while_node.line_number = original->while_node.line_number;
            clone->while_node.body = cloneAST(original->while_node.body);
            clone->while_node.condition_expr = cloneAST(original->while_node.condition_expr);
            clone->while_node.label = cloneLabel(original->while_node.label);
            break;
        }

        case NODE_IF:
        {
            // printf("CLONING IF BLOCK\n");
            clone->if_node.line_number = original->if_node.line_number;

            clone->if_node.body = cloneAST(original->if_node.body);
            clone->if_node.condition_expr = cloneAST(original->if_node.condition_expr);
            clone->if_node.condition_size = original->if_node.condition_size;
            clone->if_node.elseif_count = original->if_node.elseif_count;
            clone->if_node.elseif_nodes = calloc(1, sizeof(ASTNode *) * clone->if_node.elseif_capacity);

            // printf("CLONING ELSEIFS BLOCKS\n");
            for (int i = 0; i < original->if_node.elseif_count; i++){
                // printf("Cloning elseif: #%i\n",i);
                clone->if_node.elseif_nodes[i] = cloneAST(original->if_node.elseif_nodes[i]);
                // printf("Finished elseif: #%i\n",i);
            }
            // printf("FINISHED ALL ELSEIFS\n");
            clone->if_node.else_body = NULL;
            if (original->if_node.else_body != NULL){
                clone->if_node.else_body = cloneAST(original->if_node.else_body);
            }
            clone->if_node.reference_label = cloneLabel(original->if_node.reference_label);
            break;
        }
        case NODE_ELSEIF:
        {
            // printf("Cloning elseif\n");
            clone->elseif_node.line_number = original->elseif_node.line_number;
            clone->elseif_node.condition_expr = cloneAST(original->elseif_node.condition_expr);
            clone->elseif_node.condition_size = original->elseif_node.condition_size;
            clone->elseif_node.body = cloneAST(original->elseif_node.body);
            // printf("Finished cloning elseif\n");
            break;
        }

        case NODE_FOR:
        {
            clone->for_node.line_number = original->for_node.line_number;
            clone->for_node.assignment_expr = cloneAST(original->for_node.assignment_expr);
            clone->for_node.body = cloneAST(original->for_node.body);
            clone->for_node.condition_expr = cloneAST(original->for_node.condition_expr);
            clone->for_node.reassignment_expr = cloneAST(original->for_node.reassignment_expr);
            clone->for_node.label = cloneLabel(original->for_node.label);
            break;
        }

        case NODE_FOREACH:
        {
            clone->foreach_node.line_number = original->foreach_node.line_number;

            clone->foreach_node.body = cloneAST(original->foreach_node.body);
            clone->foreach_node.counter_name = mystrdup(original->foreach_node.counter_name);
            clone->foreach_node.foreach_declaration = cloneAST(original->foreach_node.foreach_declaration);

            // NULL for foreach lowered?
            clone->foreach_node.foreach_lowered = NULL;

            clone->foreach_node.iterable_expr = cloneAST(original->foreach_node.iterable_expr);
            clone->foreach_node.iterable_expr_type = original->foreach_node.iterable_expr_type;
            clone->foreach_node.iterator_name = mystrdup(original->foreach_node.iterator_name);
            clone->foreach_node.iterator_type = original->foreach_node.iterator_type;
            clone->foreach_node.lower_limit_expr = cloneAST(original->foreach_node.lower_limit_expr);
            clone->foreach_node.upper_limit_expr = cloneAST(original->foreach_node.upper_limit_expr);

            clone->foreach_node.label = cloneLabel(original->foreach_node.label);
            break;
        }


        case NODE_FUNC_CALL:
        {

            clone->funccall_node.line_number = original->funccall_node.line_number;

            clone->funccall_node.identifier = mystrdup(original->funccall_node.identifier);
            clone->funccall_node.params_count = original->funccall_node.params_count;
            
            clone->funccall_node.params_expr = malloc(sizeof(ASTNode *) * clone->funccall_node.params_count);
            for (int i = 0; i < clone->funccall_node.params_count; i++)
            {
                clone->funccall_node.params_expr[i] = cloneAST(original->funccall_node.params_expr[i]);
            }
            
            clone->funccall_node.params_type = malloc(sizeof(char *) * clone->funccall_node.params_count);
            for (int i = 0; i < clone->funccall_node.params_count; i++)
            {
                clone->funccall_node.params_type[i] = original->funccall_node.params_type[i];
            }

            clone->funccall_node.function = original->funccall_node.function;
            break;
        }

        case NODE_PTR_METHOD_DISPATCH:
        {
            clone->ptr_method_dispatch.base = cloneAST(original->ptr_method_dispatch.base);
            clone->ptr_method_dispatch.base_type = original->ptr_method_dispatch.base_type;
            clone->ptr_method_dispatch.func_call = cloneAST(original->ptr_method_dispatch.func_call);
            clone->ptr_method_dispatch.line_number = original->ptr_method_dispatch.line_number;
            if (clone->ptr_method_dispatch.ptrmd_lowered == NULL){
                clone->ptr_method_dispatch.ptrmd_lowered = NULL;
            }
            else{
                clone->ptr_method_dispatch.ptrmd_lowered = cloneAST(original->ptr_method_dispatch.ptrmd_lowered);
            }
            break;
        }

        case NODE_METHOD_DISPATCH:
        {
            clone->method_dispatch.base = cloneAST(original->method_dispatch.base);
            clone->method_dispatch.base_type = original->method_dispatch.base_type;
            clone->method_dispatch.func_call = cloneAST(original->method_dispatch.func_call);
            clone->method_dispatch.line_number = original->method_dispatch.line_number;
            if (clone->method_dispatch.md_lowered == NULL){
                clone->method_dispatch.md_lowered = NULL;
            }
            else{
                clone->method_dispatch.md_lowered = cloneAST(original->method_dispatch.md_lowered);
            }
            break;
        }


        case NODE_RETURN:
        {
            clone->return_node.line_number = original->return_node.line_number;
            clone->return_node.return_expr = NULL;
            if (original->return_node.return_expr != NULL)
            {
                clone->return_node.return_expr = cloneAST(original->return_node.return_expr);
            }
            break;
        }



        default:
        {
            fprintf(stderr, "Unknown AST node to clone: %s\n", astTypeToStr(original));
            exit(1);
        }
    }
    return clone;
}



ASTNode *lowerForeach(ASTNode *ast_foreach)
{

    /*
    for (uint64 counter = L; counter < R; counter++)
    {
        Type iterator = iteratable_expr[counter]
        <body>
    }
    */
    // printf("LOWERING FOREACH\n");

    char *iterator_name = strdup(ast_foreach->foreach_node.foreach_declaration->declaration_node.identifier);

    /**
     * uint64 counter = lower_limit;
     */
    char counter_name[64] = {0};
    snprintf(counter_name, sizeof(counter_name) ,"__foreach_counter_%i", getCurrentContextBlockId(CTX_FOREACH));
    Type *counter_type = createNumberTypeFromInfo(8,1, NUMBER_INTEGER);
    ASTNode *ast_counter_assignment = makeAssignment(counter_type, counter_name, ast_foreach->foreach_node.lower_limit_expr);

    /**
     * counter < R
     */
    ASTNode *counter_id_binop = makeIdentifier(counter_name);
    ASTNode *condition = makeBinOP(counter_id_binop, ast_foreach->foreach_node.upper_limit_expr, "<");

    /**
     * counter++;
     */

    ASTNode *counter_id_incr = makeIdentifier(counter_name);
    ASTNode *ast_incr_counter = makePostFix(counter_id_incr, counter_type, 8, "++");
    ast_incr_counter->is_stmt = 1;

    /**
     * Type iterator = iterable_expr[counter]
     */

    ASTNode *counter_id_subscript = makeIdentifier(counter_name);
    ASTNode *ast_subscript = makeSubscript(ast_foreach->foreach_node.iterable_expr, counter_id_subscript, ast_foreach->foreach_node.iterable_expr_type,
                                            calculateSizeOfType(ast_foreach->foreach_node.iterator_type),8);

    ASTNode *iterator_assignemnt = makeAssignment(ast_foreach->foreach_node.iterator_type, iterator_name, ast_subscript);


    int n_foreach_statements = ast_foreach->foreach_node.body->block_node.statement_count;
    ASTNode **for_statements = malloc(sizeof(ASTNode*) * (n_foreach_statements + 1));

    for_statements[0] = iterator_assignemnt;
    for (int i = 1; i < n_foreach_statements + 1; i++){
        // printf("Cloning foreach statement: %i\n", i);
        for_statements[i] = cloneAST(ast_foreach->foreach_node.body->block_node.statements[i - 1]);
        // printf("Finished cloning foreach statement: %i\n",i);
    }

    ASTNode *ast_for_block = makeBlock(for_statements, n_foreach_statements + 1, n_foreach_statements + 1);
    ASTNode *ast_for = makeFor(ast_counter_assignment, condition, ast_incr_counter, ast_for_block);

    // printf("FINISHED LOWERING FOREACH\n");
    // print_ast(ast_for, 0);
    return ast_for;
}


ASTNode *lowerMethodDispatch(ASTNode *ast_md)
{
    /**
     * Get the base := base.method()
     */
    ASTNode *instance_base = cloneAST(ast_md->method_dispatch.base);


    /**
     * Need the address of the base
     */
    
    ASTNode *address_of_base = makeUnaryOP(instance_base, "&", calculateSizeOfType(ast_md->method_dispatch.base_type));

    /**
     * Create a func call := method(base, arg1, arg2...)
     */

    // Copy the arguments expression. First expression is base 
    // Copy the types as well
    ASTNode **arg_exprs = malloc(sizeof(ASTNode *) * ast_md->method_dispatch.func_call->funccall_node.params_count + 1);
    arg_exprs[0] = address_of_base;
    Type **arg_types = malloc(sizeof(Type *) * ast_md->method_dispatch.func_call->funccall_node.params_count + 1);
    arg_types[0] = ast_md->method_dispatch.base_type;

    for (int i = 1; i <= ast_md->method_dispatch.func_call->funccall_node.params_count; i++)
    {

        arg_exprs[i] = cloneAST(ast_md->method_dispatch.func_call->funccall_node.params_expr[i - 1]);

        // arg_types is populated during the analysis, which can happen after calling lower(), therefore
        // types are all NULL still, but will be populated later. Same thing happens with NODE_NUMBER for the int_type during
        // cloneAST()
 
        arg_types[i] = ast_md->method_dispatch.func_call->funccall_node.params_type[i - 1];
        

    }


    ASTNode *func_call = makeFuncCall(ast_md->method_dispatch.func_call->funccall_node.identifier, 
                                      ast_md->method_dispatch.func_call->funccall_node.params_count + 1, 
                                      arg_exprs, arg_types);    
    return func_call;
}

ASTNode *lowerPtrMethodDispatch(ASTNode *ast_ptrmd)
{

    /**
     * Get the base := base.method()
     */
    ASTNode *instance_base = cloneAST(ast_ptrmd->ptr_method_dispatch.base);


    /**
     * Create a func call := method(base, arg1, arg2...)
     */
    
    // Copy the arguments expression. First expression is base 
    // Copy the types as well
    ASTNode **arg_exprs = malloc(sizeof(ASTNode *) * ast_ptrmd->ptr_method_dispatch.func_call->funccall_node.params_count + 1);
    arg_exprs[0] = instance_base;
    Type **arg_types = malloc(sizeof(Type *) * ast_ptrmd->ptr_method_dispatch.func_call->funccall_node.params_count + 1);
    arg_types[0] = ast_ptrmd->ptr_method_dispatch.base_type;

    for (int i = 1; i <= ast_ptrmd->ptr_method_dispatch.func_call->funccall_node.params_count; i++)
    {
        arg_exprs[i] = cloneAST(ast_ptrmd->ptr_method_dispatch.func_call->funccall_node.params_expr[i - 1]);
        arg_types[i] = ast_ptrmd->ptr_method_dispatch.func_call->funccall_node.params_type[i - 1];
    }


    ASTNode *func_call = makeFuncCall(ast_ptrmd->ptr_method_dispatch.func_call->funccall_node.identifier, 
                                      ast_ptrmd->ptr_method_dispatch.func_call->funccall_node.params_count + 1, 
                                      arg_exprs, arg_types);

    return func_call;
}

/**
 * ===============================================================================
 * END LOWERERS     
 * ===============================================================================   
 */


/**
 * ===============================================================================
 * BINARY, UNARY AND POSTFIX OPERATIONS     
 * ===============================================================================   
 */

char *binopcodeToStr(BinOPCode b_op)
{
    switch(b_op)
    {
        case ADD: return "+";
        case SUB: return "-";
        case MUL: return "*";
        case DIV: return "/";
        case GT: return ">";
        case MOD: return "%";
        case LT: return "<";
        case GTEQ: return ">=";
        case LTEQ: return "<=";
        case EQ: return "==";
        case NEQ: return "!-";
        case BIT_AND: return "&";
        case BIT_OR: return "|";
        case AND: return "&&";
        case OR: return "||";
        case SHIFT_R: return ">>";
        case SHIFT_L: return "<<";
        case UNKNOWN: return "UNKNOWN";
    }

    fprintf(stderr, "Unknown binary operation code to turn to str: %i\n", b_op);
    exit(1);
}

BinOPCode strToBinOPCode(char *str)
{
    if (strcmp(str, "+") == 0) {return ADD;}
    if (strcmp(str, "-") == 0) {return SUB;}
    if (strcmp(str, "*") == 0) {return MUL;}
    if (strcmp(str, "%") == 0) {return MOD;}
    if (strcmp(str, "/") == 0) {return DIV;}
    if (strcmp(str, ">") == 0) {return GT;}
    if (strcmp(str, "<") == 0) {return LT;}
    if (strcmp(str, ">=") == 0) {return GTEQ;}
    if (strcmp(str, "<=") == 0) {return LTEQ;}
    if (strcmp(str, "==") == 0) {return EQ;}
    if (strcmp(str, "!=") == 0) {return NEQ;}
    if (strcmp(str, "&") == 0)  { return BIT_AND; }
    if (strcmp(str, "|") == 0) { return BIT_OR; }
    if (strcmp(str, "&&") == 0) {return AND;}
    if (strcmp(str, "||") == 0) {return OR;}
    if (strcmp(str, ">>") == 0) { return SHIFT_R; }
    if (strcmp(str, "<<") == 0) { return SHIFT_L; }

    return UNKNOWN;
}

BinOPCode tokenAssignToBinOPCode(TokenType tt)
{
    switch(tt)
    {
        case TOKEN_ADD_ASSIGN: return ADD;
        case TOKEN_SUB_ASSIGN: return SUB;
        case TOKEN_DIV_ASSIGN: return DIV;
        case TOKEN_MUL_ASSIGN: return MUL;
        case TOKEN_MOD_ASSIGN: return MOD;
        default: return UNKNOWN;
    }
}

UnaryOPCode strToUnaryOPCode(char *str)
{
    if (strcmp(str, "-") == 0) { return UNARY_MINUS; }
    if (strcmp(str, "*") == 0) { return UNARY_DEREF; }
    if (strcmp(str, "&") == 0) { return UNARY_ADDRESS; }
    if (strcmp(str, "!") == 0) { return UNARY_NOT; }

    return UNARY_UNKNOWN;
}

PostfixOPCode strToPostfixOPCode(char *str)
{
    if (strcmp(str, "++") == 0) { return POSTFIX_INC; }
    if (strcmp(str, "--") == 0) { return POSTFIX_DEC; }

    fprintf(stderr, "Operation: '%s', is not recognized as a postfix operation \n", str);
    exit(1);
}


/**
 * ===============================================================================
 * END BINARY, UNARY AND POSTFIX OPERATIONS     
 * ===============================================================================   
 */

/**
 * ===============================================================================
 * TYPE OPERATIONS     
 * ===============================================================================   
 */


char *shortForType(Type *type)
{
    //printf("Trying to write short for type: %s\n", typeToString(type));

    char *buffer = malloc(256);
    char *ptr = buffer;
    switch(type->kind)
    {
        case TYPE_STR:{
            ptr += sprintf(ptr, "pc");
            return buffer;
        }
        case TYPE_VOID:{
            return buffer;
        }
        case TYPE_ARRAY:{
            fprintf(stderr, "Cant get short for type array, cant have it as parameter so..\n");
            exit(1);
        }
        case TYPE_NUMBER:{
            switch(type->numberType.kind){
                case NUMBER_BOOL:{
                    sprintf(ptr, "b");
                    break;
                }
                case NUMBER_CHAR:{
                    sprintf(ptr, "c");
                    break;
                }
                case NUMBER_INTEGER:{
                    if (type->numberType.is_unsigned){
                        ptr += sprintf(ptr, "u");
                    }
                    else{
                        ptr += sprintf(ptr, "i");
                    }
                    ptr += sprintf(ptr, "%i", type->numberType.bytes * 8);
                    break;
                }
            }
            return buffer;
        }
        case TYPE_PTR:{
            ptr += sprintf(ptr, "p");
            char *inner_type = shortForType(type->pointerType.pointed_type);
            ptr += sprintf(ptr, "%s", inner_type);
            return buffer;
        }
        case TYPE_UDT:{
            ptr += sprintf(ptr, "%s", type->udt.name);
            return buffer;
        }
    }

    return NULL;
}

int isTypeAbleToSubscript(Type *type)
{
    switch(type->kind)
    {
        case TYPE_ARRAY:
        case TYPE_STR:
        case TYPE_PTR: return 1;
        default: return 0;
    }
}

int generationLevelsBetweenTypes(Type *t1, Type *t2)
{
    if (t1->kind != TYPE_UDT || t2->kind != TYPE_UDT){
        return 0;
    }

    switch (t1->udt.udt_kind)
    {
        case UDT_CLASS:{
            Class *c1 = lookUpClass(gb_classTable, t1->udt.name);
            Class *c2 = lookUpClass(gb_classTable, t2->udt.name);

            int c1_count = 0;
            int c2_count = 0;

            /** Check c1 first */
            Class *curr = c1;
            while (curr != NULL){
                if (strcmp(curr->identifier, c2->identifier) == 0){
                    break;
                }
                c1_count++;
                curr = curr->parent;
            }
            /** Now c2 */
            curr = c2;
            while (curr != NULL){
                if (strcmp(curr->identifier, c1->identifier) == 0){
                    break;
                }
                c2_count++;
                curr = curr->parent;
            }

            /** Return the minimum, one of the two may have been the parent and that escales up to null */
            if (c1_count > c2_count){
                return c2_count;
            }
            else{
                return c1_count;
            }
        }
        case UDT_OBJECT:{
            Object *o1 = lookUpObject(gb_objectTable, t1->udt.name);
            Object *o2 = lookUpObject(gb_objectTable, t2->udt.name);

            int o1_count = 0;
            int o2_count = 0;

            /** Check o1 first */
            Object *curr = o1;
            while (curr != NULL){
                if (strcmp(curr->identifier, o2->identifier) == 0){
                    break;
                }
                o1_count++;
                curr = curr->parent;
            }
            /** Now o2 */
            curr = o2;
            while (curr != NULL){
                if (strcmp(curr->identifier, o1->identifier) == 0){
                    break;
                }
                o2_count++;
                curr = curr->parent;
            }

            /** Return the minimum, one of the two may have been the parent and that escales up to null */
            if (o1_count > o2_count){
                return o2_count;
            }
            else{
                return o1_count;
            }
        }
        default:{
            return 0;
        }
    }

}

int isUDTOfKind(UserDefinedType *udt, UserDefinedTypeKind udt_kind){
    return udt->udt_kind == udt_kind;
}


int calculateSizeOfType(Type *type)
{

    if (type == NULL){
        fprintf(stderr, "Cant calculate the size of a NULL type\n");
        exit(1);
    }

    //printf("Calculating the size of type: %s\n", typeToString(type));

    switch(type->kind)
    {
        case TYPE_NUMBER:{
            return type->numberType.bytes;
        }
        case TYPE_PTR:
        case TYPE_STR:{
            return 8;
        }

        case TYPE_ARRAY:{
            return type->arrayType.n_of_elements * calculateSizeOfType(type->arrayType.element_type);
        }

        case TYPE_UDT:
        {
            int size = 0;
            switch(type->udt.udt_kind)
            {
                case UDT_CLASS:
                {
                    Class *c = lookUpClass(gb_classTable, type->udt.name);
                    for (int i = 0; i < c->local_symbols->count; i++){
                        size += calculateSizeOfType(c->local_symbols->symbols[i]->smi->type);
                    }
                    size += c->local_symbols->padding;

                    int total_size = align_to(size, type);  
                    return total_size;
                }
                case UDT_OBJECT:
                {
                    Object *o = lookUpObject(gb_objectTable, type->udt.name);
                    for (int i = 0; i < o->local_symbols->count; i++){
                        size += calculateSizeOfType(o->local_symbols->symbols[i]->smi->type);
                    }
                    size += o->local_symbols->padding;

                    int total_size = align_to(size, type);  

                    return total_size;
                }
                case UDT_ENUM:{
                    Enum *e = lookUpEnum(gb_enumTable, type->udt.name);
                    int size = calculateSizeOfType(e->base_type);
                    return size;
                }
                case UDT_UNKNOWN_YET:{
                    fprintf(stderr, "Can't calculate size of type: %s\n", typeToString(type));
                    exit(1);
                }
            }
        }

        default:{
            fprintf(stderr, "Can't calculate size of type: %s\n", typeToString(type));
            exit(1);
        }
    }
}


int typesMatchForFunctionCall(Type *type, Type *providedType)
{

    if (type->kind != providedType->kind){
        return 0;
    }

    switch(type->kind){
        case TYPE_VOID:{
            return 1;
        }
        case TYPE_ARRAY:{
            return 0;
        }
        case TYPE_NUMBER:{

            if (providedType->numberType.kind != type->numberType.kind){
                return 0;
            }
                        /**
             * If provided number is bigger, not the function we look for
             */
            if (providedType->numberType.bytes > type->numberType.bytes){
                return 0;
            }

            /**
             * If provided and defined have differenty signedness, not the function we look  for
             */
            // if (providedType->numberType.is_unsigned != type->numberType.is_unsigned){
            //     return 0;
            // }


            return 1;
            // return (type->numberType.bytes == providedType->numberType.bytes && type->numberType.is_unsigned == providedType->numberType.is_unsigned &&
            //         type->numberType.kind == providedType->numberType.kind);
        }
        case TYPE_STR:{
            return 1;
        }
        case TYPE_PTR:{
            Type *declaredPointedType = type->pointerType.pointed_type;
            Type *assigningPointedType = providedType->pointerType.pointed_type;
                
            if (isTypeOfKind(assigningPointedType, TYPE_VOID) || isTypeOfKind(declaredPointedType, TYPE_VOID)){
                return 1;
            }
            return typesMatchForFunctionCall(type->pointerType.pointed_type, providedType->pointerType.pointed_type);
        }
        case TYPE_UDT:{
            if (type->udt.udt_kind != providedType->udt.udt_kind){
                return 0;
            }
            switch(type->udt.udt_kind)
            {
                case UDT_ENUM:{
                    return (strcmp(type->udt.name, providedType->udt.name) == 0);
                }
                case UDT_OBJECT:{
                    Object *providedObject = lookUpObject(gb_objectTable, providedType->udt.name);
                    Object *currentObject = providedObject;
                    while (currentObject != NULL){
                        if (strcmp(currentObject->identifier, type->udt.name) == 0){
                            return 1;
                        }
                        currentObject = currentObject->parent;
                    }
                    return 0;
                }
                case UDT_CLASS:{
                    Class *providedClass = lookUpClass(gb_classTable, providedType->udt.name);
                    Class *currentClass = providedClass;
                    while (currentClass != NULL){
                        if (strcmp(currentClass->identifier, type->udt.name) == 0){
                            return 1;
                        }
                        currentClass = currentClass->parent;
                    }
                    return 0;
                }
                case UDT_UNKNOWN_YET:{
                    fprintf(stderr, "Can't check whether a UDT unkown yet type matches for func call.\n");
                    exit(1);
                }
            }
        }
        default:{
            fprintf(stderr, "Unknown type to check match for func call.\n");
            exit(1);
        }
    }
}

int isTypeAssignableToDeclared(Type *declaredType, Type *assigningType)
{

    //printf("declared: %s, assigning: %s\n", typeToString(declaredType), typeToString(assigningType));
    if (declaredType == NULL || assigningType == NULL){
        fprintf(stderr, "Can't check whether type is assignable to declared. one is null\n");
    }

    if (declaredType == assigningType){
        return 1;
    }

    /**
     * If they have different kind, they cannot be assigned. 
     */
    if (declaredType->kind != assigningType->kind){
        return 0;
    }

    /**
     * At this point they have the same kind
     */
    switch(assigningType->kind)
    {
        case TYPE_VOID: {
            return 1;
        }
        case TYPE_NUMBER:
        {

            /**
             * If assignning number is bigger, there is overflow: Not assignable
             */
            if (assigningType->numberType.bytes > declaredType->numberType.bytes){
                return 0;
            }

            /**
             * If assignning is signed but declared is unsigned: Not assignable
             */
            // if (!assigningType->numberType.is_unsigned && declaredType->numberType.is_unsigned){
            //     return 0;
            // }

            /**
             * If declared is signed and assigning is > 
             */

            return 1;
        }
        
        case TYPE_ARRAY:
        {
            /**
             * WHAT DO WE DO HERE
             */
            // if (assigningType->arrayType.n_of_elements != declaredType->arrayType.n_of_elements){
            //     return 0;
            // }

            // return isTypeAssignableToDeclared(assigningType->arrayType.element_type, declaredType->arrayType.element_type);
            return 0;
        }

        case TYPE_PTR:
        {

            /**
             * At this point none of them point to void, they have to point to the same
             */
            Type *declaredPointedType = declaredType->pointerType.pointed_type;
            Type *assigningPointedType = assigningType->pointerType.pointed_type;

            /** If assining is ptr<void>, it is assignable */
            if (isTypeOfKind(assigningPointedType, TYPE_VOID) || isTypeOfKind(declaredPointedType, TYPE_VOID)){
                return 1;
            }

            return isTypeAssignableToDeclared(declaredPointedType, assigningPointedType);
            
        }
        case TYPE_UDT:
        {
            /**
             * If they are not the same UDT kind, is not assignable
             */
            if (assigningType->udt.udt_kind != declaredType->udt.udt_kind){
                return 0;
            }
            /**
             * Different name, not assignable. If B extends A, and we do b = a; we can try to later on access b fields that are NOT in a
             */

            if (strcmp(assigningType->udt.name, declaredType->udt.name) != 0){
                return 0;
            }
            return 1;  
        }


        default:
        {
            fprintf(stderr, "Unknown TYPES to check assignability\n");
            exit(1);
        }
    }
}

void validateBinaryOP(Type *left, Type *right, BinOPCode bin_op)
{

    //printf("Validating bin op between types: %s, %s\n", typeToString(left), typeToString(right));

    /**
     * (Num || enum) [bin_op] (Num || enum), everything's supported
     */
    if ( (left->kind == TYPE_NUMBER || (left->kind == TYPE_UDT && left->udt.udt_kind == UDT_ENUM)) && 
          (right->kind == TYPE_NUMBER || (left->kind == TYPE_UDT && right->udt.udt_kind == UDT_ENUM))){ 
        return; 
    }

    /**
     * (ptr [bin_op] Num) supported only for "+" and "-"
     */
    if ( (left->kind == TYPE_PTR && right->kind == TYPE_NUMBER) || 
        ( left->kind == TYPE_NUMBER && right->kind == TYPE_PTR )){
        switch (bin_op)
        {
            case ADD:
            case SUB: return;

            default: break;
        }
    }

    /** ptr [bin_op] ptr allowed for == and != */
    if (isTypeOfKind(left, TYPE_PTR) && isTypeOfKind(right, TYPE_PTR)){
        switch (bin_op){
            case AND:
            case OR:
            case GT:
            case LT:
            case GTEQ:
            case LTEQ:
            case EQ:
            case NEQ: return;

            default: break;
        }
    }

    /**
     * Not supported
     */
    Error *e = invoke(BINARYOP_NOT_TRIVIAL);
    appendMessageToError(e, fetchErrorMsgBasedOnErrorID(e->error_id), binopcodeToStr(bin_op), typeToString(left), typeToString(right));
    addError(e);
}

void validateUnaryOP(Type *right, char *unary_op)
{
    UnaryOPCode unary_code = strToUnaryOPCode(unary_op);
    switch(unary_code)
    {
        case UNARY_MINUS:
        case UNARY_NOT:
        {
            if (right->kind == TYPE_NUMBER){
                return;
            }
            break;
        }
        case UNARY_ADDRESS:
        {
            if (right->kind != TYPE_VOID){
                return;
            }
            break;            
        }
        case UNARY_DEREF:
        {
            if (right->kind == TYPE_PTR || right->kind == TYPE_STR){
                return;
            }
            break;
        }

        default:{
            break;
        }
    }


    Error *e = invoke(UNARYOP_INVALID);
    appendMessageToError(e, fetchErrorMsgBasedOnErrorID(e->error_id), typeToString(right), unary_op);
    addError(e);
}

Type *performUnaryOnType(Type *right, char *unary_op)
{
    UnaryOPCode unary_code = strToUnaryOPCode(unary_op);


    switch(unary_code)
    {
        case UNARY_MINUS:{
            Type *result = createNumberTypeFromInfo(right->numberType.bytes, 0, NUMBER_INTEGER);
            return result;
        }
        case UNARY_NOT:{
            return right;
        }
        case UNARY_ADDRESS:{
            Type *result = createPointerTypeFromInfo(right);
            return result;
        }
        case UNARY_DEREF:{
            return right->pointerType.pointed_type;
        }

        default: {
            return NULL;
        }
    }
}

Type *performBinaryOnTypes(Type *left, Type *right, BinOPCode bin_code)
{
    switch(bin_code)
    {
        case AND:
        case OR:
        case GT:
        case GTEQ:
        case LT:
        case LTEQ:
        case EQ:
        case NEQ:{
            // create a uint8 holding the result
            Type *result = createNumberTypeFromInfo(1,1, NUMBER_INTEGER);
            return result;
        }
        
        default:{
            int lsize = calculateSizeOfType(left);
            int rsize = calculateSizeOfType(right);

            if (lsize > rsize){
                return left;
            }
            else if (rsize > lsize){
                return right;
            }
            else{
                // Check if one is a pointer, if so, return that one
                if (isTypeOfKind(left, TYPE_PTR)){
                    return left;
                }
                if (isTypeOfKind(right, TYPE_PTR)){
                    return right;
                }
                char l_is_signed = isTypeUnsignedInt(left);
                char r_is_signed = isTypeUnsignedInt(right);
                // None is a pointer and sizes are equal, prefer signed if one is signed
                if (l_is_signed != r_is_signed){
                    // Promote to signed
                    if (l_is_signed)  { return left; }
                    return right;
                }

                // Both same signedness, return either
                return left;
            }
        }

    }
}

void validateReassignOperationWithTypes(Type *lvalue, Type *rvalue, TokenType reassign_symbol)
{
    switch(reassign_symbol)
    {
        case TOKEN_ASSIGN: 
        {
            int match = isTypeAssignableToDeclared(lvalue, rvalue);
            if (!match){
                Error *e = invoke(RTYPE_CANT_BE_ASSIGNED_TO_LTYPE);
                appendMessageToError(e, fetchErrorMsgBasedOnErrorID(e->error_id), typeToString(lvalue), typeToString(rvalue));
                addError(e);
            }
            return;
        }

        /**
         * Basically now we have lvalue = lvalue + rvalue ---> The binary op must be supported
         */
        default:
        {
            BinOPCode op_bin = tokenAssignToBinOPCode(reassign_symbol);
            validateBinaryOP(lvalue, rvalue, op_bin);
        }

    }
}

int doesTypeExist(Type *type)
{
    // printf("Checking if type at %p exists, %s\n", type, type->representation);

    Type *base = getBaseType(type);

    // printf("Checking if base at %p exists, %s\n", base, base->representation);


    // If base is UDT UNKNOWN, type is undefined yet for usage
    if (base->kind == TYPE_UDT && base->udt.udt_kind == UDT_UNKNOWN_YET){
        // printf("Fell here, %s\n", base->udt.name);
        Error *e = invoke(TYPE_UNDEFINED);
        appendMessageToError(e, fetchErrorMsgBasedOnErrorID(e->error_id), typeToString(base));
        addError(e);
    }

    // printf("Checking, is type in table? %i\n", isTypeInTable(type));
    return isTypeInTable(type);
}

int checkConflictingUDTs(char *udt_name, UserDefinedTypeKind udt_kind, void *udt)
{

    int gotta_add = 1;

    for (int i = 0; i < gb_classTable->count; i++){
        if (strcmp(gb_classTable->classes[i]->identifier, udt_name) == 0){
            gotta_add = 0;

            // Found the actual def of the forwarded class, swap it
            if (udt_kind == UDT_CLASS && gb_classTable->classes[i]->is_forward == 1){
                gb_classTable->classes[i] = (Class *)udt;
                break;
            }
            if (udt_kind == UDT_CLASS){
                Class *udt_class = (Class *)udt;
                if (udt_class->is_forward == 1){
                    // There is already a class completely defined in the table, ignore the forward decl
                    break;
                }
            }
            Error *e = invoke(REDEFINITION_OF_UDT);
            appendMessageToError(e, fetchErrorMsgBasedOnErrorID(e->error_id), udt_name);
            appendMessageToError(e, "There is a Class with that name already defined in file: '%s', in L = %i.", 
                                 gb_classTable->classes[i]->def_src_file, gb_classTable->classes[i]->def_line_number);
            addError(e);
        }
    }

    for (int i = 0; i < gb_objectTable->count; i++){
        if (strcmp(gb_objectTable->objects[i]->identifier, udt_name) == 0){
            gotta_add = 0;

            // Found the actual def of the forwarded object, swap it
            if (udt_kind == UDT_OBJECT && gb_objectTable->objects[i]->is_forward == 1){
                gb_objectTable->objects[i] = (Object *)udt;
                break;
            }
            if (udt_kind == UDT_OBJECT){
                Object *udt_object = (Object *)udt;
                if (udt_object->is_forward == 1){
                    // There is already an object completely defined in the table, ignore the forward decl
                    break;
                }
            }
            Error *e = invoke(REDEFINITION_OF_UDT);
            appendMessageToError(e, fetchErrorMsgBasedOnErrorID(e->error_id), udt_name);
            appendMessageToError(e, "There is an Object with that name already defined in file: '%s', in L = %i.", 
                                 gb_objectTable->objects[i]->def_src_file, gb_objectTable->objects[i]->def_line_number);
            addError(e);
        }
    }

    
    for (int i = 0; i < gb_enumTable->count; i++){
        if (strcmp(gb_enumTable->enums[i]->identifier, udt_name) == 0){
            gotta_add = 0;
            // Found the actual def of the forwarded enum, swap it
            if (udt_kind == UDT_ENUM && gb_enumTable->enums[i]->is_forward == 1){
                gb_enumTable->enums[i] = (Enum *)udt;
                break;
            }
            if (udt_kind == UDT_ENUM){
                Enum *udt_enum = (Enum *)udt;
                if (udt_enum->is_forward == 1){
                    // There is already an enum completely defined in the table, ignore the forward decl
                    break;
                }
            }
            Error *e = invoke(REDEFINITION_OF_UDT);
            appendMessageToError(e, fetchErrorMsgBasedOnErrorID(e->error_id), udt_name);
            appendMessageToError(e, "There is an Enum with that name already defined in file: '%s', in L = %i.", 
                                 gb_enumTable->enums[i]->def_src_file, gb_enumTable->enums[i]->def_line_number);
            addError(e);
        }
    }

    return gotta_add;

}

void populateUDTInfo(UserDefinedTypeKind udt_kind, char *udt_name)
{
    //printf("Populating udt info for name: %s\n", udt_name);
    for (int i = 0; i < gb_typeTable->count; i++){
        switch(gb_typeTable->types[i]->kind)
        {
            case TYPE_ARRAY:{
                Type *element = gb_typeTable->types[i]->arrayType.element_type;
                if (element->kind == TYPE_UDT && strcmp(element->udt.name, udt_name) == 0){
                    gb_typeTable->types[i]->representation = updateTypeRepresentation(gb_typeTable->types[i]);
                }
                break;
            }
            case TYPE_PTR:{
                Type *t = getFinalPointedType(gb_typeTable->types[i]);
                if (t->kind == TYPE_UDT && strcmp(t->udt.name, udt_name) == 0){
                    gb_typeTable->types[i]->representation = updateTypeRepresentation(gb_typeTable->types[i]);
                }
                break;
            }
            case TYPE_UDT:{
                if (strcmp(gb_typeTable->types[i]->udt.name, udt_name) == 0){
                    gb_typeTable->types[i]->representation = updateTypeRepresentation(gb_typeTable->types[i]);
                }
                break;
            }
            default:{
                break;
            }
        }
    }

}



/**
 * ===============================================================================
 * END TYPE OPERATIONS     
 * ===============================================================================   
 */



/**
 * ===============================================================================
 * STRING TABLE OPERATIONS     
 * ===============================================================================   
 */


StringTable *__allocateStringTable()
{
    StringTable *str_table = malloc(sizeof(StringTable));
    str_table->capacity = STRINGTABLE_INIT_CAPACITY;
    str_table->saved_strings = malloc(STRINGTABLE_INIT_CAPACITY * sizeof(char *));
    str_table->labels = malloc(STRINGTABLE_INIT_CAPACITY * sizeof(Label *));

    return str_table;
}

void __resizeStringTable(StringTable *str_table)
{
    str_table->capacity = STRINGTABLE_RESIZE_FACTOR * str_table->capacity;
    str_table->saved_strings = realloc(str_table->saved_strings, str_table->capacity * sizeof(char *));
    str_table->labels = realloc(str_table->labels, str_table->capacity * sizeof(Label *));
}

StringTable *makeStringTable()
{
    StringTable *str_table = __allocateStringTable();
    str_table->count = 0;

    return str_table;
}

void __initGlobalStringTable()
{
    gb_stringTable = makeStringTable();
}

void addStrToTable(StringTable *str_table, char *str)
{
    if (str_table->count + 1 >= str_table->capacity){
        __resizeStringTable(str_table);
    }

    str_table->saved_strings[str_table->count] = str;

    char tag_name[32];
    snprintf(tag_name, 32, "Lstr%i", str_table->count);

    str_table->labels[str_table->count++] = makeTagLabel(tag_name);

}

Label *getStringTableLabelAt(StringTable *str_table, int index)
{
    return str_table->labels[index];
}

 /**
 * ===============================================================================
 * END STRING TABLE OPERATIONS     
 * ===============================================================================   
 */



 
/**
 * ===============================================================================
 * LABEL OPERATIONS     
 * ===============================================================================   
 */

Tag *__allocateTag()
{
    Tag *t = malloc(sizeof(Tag));
    return t;
}

Tag *makeTag(const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);

    /**
     * Determine the required buffer size
     */
    va_list args_copy;
    va_copy(args_copy, args);
    int size = vsnprintf(NULL,0, fmt, args_copy); 
    va_end(args_copy);

    if (size < 0){
        va_end(args);
        return NULL;
    }

    char *buffer = malloc(size + 1);
    vsnprintf(buffer, size + 1, fmt, args);
    va_end(args);

    Tag *tag = __allocateTag();
    tag->tag_name = buffer;
    return tag;
}

void __resizeIntermediateTagsInCompoundLabel(Label *l)
{
    l->compound_label.intermediate_tags_capacity = INTERMEDIATE_TAG_RESIZE_FACTOR * l->compound_label.intermediate_tags_capacity;
    l->compound_label.intermediate_tags = realloc(l->compound_label.intermediate_tags, l->compound_label.intermediate_tags_capacity * sizeof(Tag *));
}

Label *__allocateLabel()
{
    Label *l = malloc(sizeof(Label));
    return l;
}

Label *makeCompoundLabel()
{
    Label *l = __allocateLabel();
    l->kind = COMPOUND;
    l->compound_label.intermediate_tags_capacity = INTERMEDIATE_TAG_INIT_CAPACITY;
    l->compound_label.intermediate_tags_count = 0;
    l->compound_label.intermediate_tags = malloc(INTERMEDIATE_TAG_INIT_CAPACITY * sizeof(Tag *));
    l->compound_label.start_tag = __allocateTag();
    l->compound_label.end_tag = __allocateTag();

    return l;
}

Label *makeTagLabel(char *tag_name)
{
    Label *l = __allocateLabel();
    l->kind = TAG;
    l->tag.tag_name = strdup(tag_name);
    return l;
}


void setStartTagToCompoundLabel(Label *l, Tag *start_tag)
{
    l->compound_label.start_tag = start_tag;
}

void setEndTagToCompoundLabel(Label *l, Tag *end_tag)
{
    l->compound_label.end_tag = end_tag;
}


void addIntermediateTagToCompoundLabel(Label *l, Tag *intermediate_tag)
{
    if (l->compound_label.intermediate_tags_count + 1 >= l->compound_label.intermediate_tags_capacity){
        __resizeIntermediateTagsInCompoundLabel(l);
    }

    l->compound_label.intermediate_tags[l->compound_label.intermediate_tags_count++] = intermediate_tag;
}

/**
 * ===============================================================================
 * END LABEL OPERATIONS     
 * ===============================================================================   
 */



/**
 * ===============================================================================
 * SYMBOL OPERATIONS     
 * ===============================================================================   
 */



 
SymbolTable *__allocateSymbolTable()
{
    SymbolTable *st = malloc(sizeof(SymbolTable));
    st->symbols = malloc(SYMBOLTABLE_INIT_CAPACITY * sizeof(Symbol *));
    st->capacity = SYMBOLTABLE_INIT_CAPACITY;
    return st;
}



void __resizeSymbolTable(SymbolTable *st)
{
    st->capacity = SYMBOLTABLE_RESIZE_FACTOR * st->capacity;
    st->symbols = realloc(st->symbols, st->capacity * sizeof(Symbol *));
}

SymbolTable *makeSymbolTable(SymbolTable *parentST, SymbolTableKind kind)
{
    SymbolTable *st = __allocateSymbolTable();
    st->count = 0;
    st->padding = 0;
    st->weight = 0;
    st->scope = getCurrentScope();
    st->parentST = parentST;
    st->kind = kind;

    return st;
}

void __initGlobalSymbolTable()
{
    gb_symbolTable = makeSymbolTable(NULL, KIND_FUNCTION_ST);
}

SymbolMetaInfo *__allocateSymbolMetaInfo()
{
    SymbolMetaInfo *smi = malloc(sizeof(SymbolMetaInfo));
    return smi;
}

Symbol *__allocateSymbol()
{
    Symbol *s = malloc(sizeof(Symbol));
    return s;
}

SymbolMetaInfo *makeSymbolMetaInfo(char *identifier, Type *type)
{

    SymbolMetaInfo *smi = __allocateSymbolMetaInfo();
    smi->identifier = identifier;
    smi->type = type;
    smi->def_line_number = tracker.current_line;
    smi->def_src_file = tracker.current_src_file;
    smi->size = calculateSizeOfType(type);
    smi->index = 0;
    smi->offset = 0;
    smi->scope = getCurrentScope();
    smi->def_ctx = getCurrentContext();
    return smi;
}

Symbol *makeGlobalSymbol(char *identifier, Type *type)
{
    Symbol *s = __allocateSymbol();
    s->symbol_kind = SYMBOL_GLOBAL;
    s->s_global.global_kind = GLOBAL_VAR;
    s->smi = makeSymbolMetaInfo(identifier, type);
    return s;
}

Symbol *makeExternSymbol(char *identifier, Type *type)
{
    Symbol *s = __allocateSymbol();
    s->symbol_kind = SYMBOL_GLOBAL;
    s->s_global.global_kind = GLOBAL_EXTERN;
    s->s_global.symbolExternVar.is_allocated = 0;
    s->smi = makeSymbolMetaInfo(identifier, type);

    //printf("Made an extern symbol with name = %s, def ctx: %s\n", identifier, ctxToString(s->smi->def_ctx.ctx_type));
    return s;
}

Symbol *makeLocalVarSymbol(char *identifier, Type *type)
{
    Symbol *s = __allocateSymbol();
    s->symbol_kind = SYMBOL_LOCAL;
    s->s_local.local_kind = LOCAL_VAR;

    s->smi = makeSymbolMetaInfo(identifier, type);
    //printf("Made a local symbol with name = %s, def ctx: %s\n", identifier, ctxToString(s->smi->def_ctx.ctx_type));

    return s;
}

Symbol *makeParameterSymbol(char *identifier, int parameter_index, Type *type)
{
    Symbol *s = __allocateSymbol();
    s->symbol_kind = SYMBOL_LOCAL;
    s->s_local.local_kind = LOCAL_PARAMETER;

    s->s_local.symbolParameter.parameter_index = parameter_index;
    s->smi = makeSymbolMetaInfo(identifier, type);

    return s;
}

void insertNewParameter(Symbol *parameterToInsert, Parameter **params, int param_count)
{
    Parameter *newp = malloc(sizeof(Parameter));
    newp->name = parameterToInsert->smi->identifier;
    newp->type = parameterToInsert->smi->type;

    int indexToInsertAt = parameterToInsert->s_local.symbolParameter.parameter_index;

    params = realloc(params, param_count + 1);

    for (int i = param_count; i >= indexToInsertAt; i--){
        params[i] = params[i - 1];
    }

    params[indexToInsertAt] = newp;

}

Symbol *lookUpSymbol(SymbolTable *st, int scope, char *identifier)
{
    // printf("-------- NEW LOOKUP. Searching for: %s ---------\n", identifier);
    // printf("[");
    // for (int i = 0; i <= context.current_scope; i++){
    //     printf("%s,",ctxToString(context.ctx_block_stack[i].ctx_type));
    // }
    // printf("]\n");
    if (identifier == NULL){
        fprintf(stderr, "Cant look up a symbol with null identifier\n");
        exit(1);
    }
    while (st != NULL){
        int checking_scope = scope;
        while (checking_scope >= 0){
            for (int i = 0; i < st->count; i++){
                Symbol *candidate = st->symbols[i];
                // printf("Looking up symbol with identifier: '%s' at scope: %i, ctx: %s, ctx_id: %i. Current candidate: '%s', with def_scope: %i, def_ctx: %s, def_ctx_id: %i\n", 
                //         identifier, checking_scope, ctxToString(context.ctx_block_stack[checking_scope].ctx_type), context.ctx_block_stack[checking_scope].block_id,
                //          candidate->smi->identifier, candidate->smi->scope, ctxToString(candidate->smi->def_ctx.ctx_type), candidate->smi->def_ctx.block_id);
                /**
                 * If not the same scope, continue (so retrieve the match in the closest scope)
                 */
                if (candidate->smi->scope != checking_scope) { 
                    continue; 
                }

                // If candidate def context is not the same context we are in, continue
                if (candidate->smi->def_ctx.ctx_type != context.ctx_block_stack[checking_scope].ctx_type){
                    continue;
                }

                // If candidate's definition block id cant be reached walking back the stack, continue
                if (candidate->smi->def_ctx.block_id != context.ctx_block_stack[checking_scope].block_id){
                    continue;
                }

                //If different name, continue
                if (strcmp(candidate->smi->identifier, identifier) != 0){
                    continue;
                }
                return candidate;

            }

            checking_scope--;
        }        
        st = st->parentST; // Move to the parent table
    }
    return NULL;   
}

Symbol *lookUpField(SymbolTable *st, char *identifier)
{
    while (st != NULL){
        for (int i = 0; i < st->count; i++){

            //If same name, retrieve it
            if (strcmp(st->symbols[i]->smi->identifier, identifier) == 0){
                return st->symbols[i];
            }
        }
        st = st->parentST; // Move to the parent table
    }

    return NULL;   
}

Symbol *lookUpParameter(SymbolTable *st, int param_index)
{
    for (int i = 0; i < st->count; i++){
        if (st->symbols[i]->symbol_kind != SYMBOL_LOCAL){
            continue;
        }
        if (st->symbols[i]->s_local.local_kind != LOCAL_PARAMETER){
            continue;
        }
        if (st->symbols[i]->s_local.symbolParameter.parameter_index == param_index){
            return st->symbols[i];
        }
    }

    return NULL;
}

void addSymbolToTable(Symbol *s, SymbolTable *st)
{
    // printf("Trying to add symbol:");
    // printf("%s", s->smi->identifier);
    // printf(",%s\n", typeToString(s->smi->type));
    /**
     * Dont allow local/global vars with the same name as globals
     * If they have the same name and at least one of them is global, not allowed
     */
    // for (int i = 0; i < gb_symbolTable->count; i++){
    //     Symbol *curr_gb_s = gb_symbolTable->symbols[i];
    //     // Different name, we good
    //     if (strcmp(curr_gb_s->smi->identifier, s->smi->identifier) != 0){
    //         continue;
    //     }
    //     // If same name and adding a local, error
    //     if (s->symbol_kind == SYMBOL_LOCAL){
    //         Error *e = invoke(REDEFINITION_OF_VARIABLE);
    //         appendMessageToError(e, fetchErrorMsgBasedOnErrorID(e->error_id), s->smi->identifier);
    //         appendMessageToError(e, "Previous definition in file: '%s', in L = %i.", curr_gb_s->smi->def_src_file, curr_gb_s->smi->def_line_number);
    //         addError(e);
    //     }

    //     // Same name, both globals
    //     // If both are regular globals or both are extern, error 
    //     if ( (curr_gb_s->s_global.global_kind == GLOBAL_VAR && s->s_global.global_kind == GLOBAL_VAR) || 
    //         (curr_gb_s->s_global.global_kind == GLOBAL_EXTERN && s->s_global.global_kind == GLOBAL_EXTERN)){
    //         Error *e = invoke(REDEFINITION_OF_VARIABLE);
    //         appendMessageToError(e, fetchErrorMsgBasedOnErrorID(e->error_id), s->smi->identifier);
    //         appendMessageToError(e, "Previous definition in file: '%s', in L = %i.", curr_gb_s->smi->def_src_file, curr_gb_s->smi->def_line_number);
    //         addError(e);
    //     }

    //     // Same name, both globals, one is regular global and the other is extern 
    //     // If the extern one hasnt been alloc'ed, this is okay 
    //     if (curr_gb_s->s_global.global_kind == GLOBAL_EXTERN){
    //         if (!curr_gb_s->s_global.symbolExternVar.is_allocated){
    //             continue;
    //         }
    //         else{
    //             Error *e = invoke(REDEFINITION_OF_VARIABLE);
    //             appendMessageToError(e, fetchErrorMsgBasedOnErrorID(e->error_id), s->smi->identifier);
    //             appendMessageToError(e, "Previous definition in file: '%s', in L = %i.", curr_gb_s->smi->def_src_file, curr_gb_s->smi->def_line_number);
    //             addError(e);
    //         }
    //     }
    //     else{
    //         if (!s->s_global.symbolExternVar.is_allocated){
    //             continue;
    //         }
    //         else{
    //             Error *e = invoke(REDEFINITION_OF_VARIABLE);
    //             appendMessageToError(e, fetchErrorMsgBasedOnErrorID(e->error_id), s->smi->identifier);
    //             appendMessageToError(e, "Previous definition in file: '%s', in L = %i.", curr_gb_s->smi->def_src_file, curr_gb_s->smi->def_line_number);
    //             addError(e);
    //         }
    //     }
    // }

    //printf("Adding symbol: %s, to table of kind: %i \n", s->smi->identifier, st->kind);
    /**
     * Don't allow duplicates in the same table
     */

    // Dont search again the gb_table
    // if (st != gb_symbolTable){
    //     for (int i = 0; i < st->count; i++){
    //         if (doSymbolsClash(s, st->symbols[i])){
    //             Error *e = invoke(REDEFINITION_OF_VARIABLE);
    //             appendMessageToError(e, fetchErrorMsgBasedOnErrorID(e->error_id), s->smi->identifier);
    //             appendMessageToError(e, "Previous definition in file: '%s', in L = %i.", st->symbols[i]->smi->def_src_file, st->symbols[i]->smi->def_line_number);
    //             addError(e);
    //         }
    //     }
    // }


    for (int i = 0; i < st->count; i++){
        if (doSymbolsClash(s, st->symbols[i])){
            Error *e = invoke(REDEFINITION_OF_VARIABLE);
            appendMessageToError(e, fetchErrorMsgBasedOnErrorID(e->error_id), s->smi->identifier);
            appendMessageToError(e, "Previous definition in file: '%s', in L = %i.", st->symbols[i]->smi->def_src_file, st->symbols[i]->smi->def_line_number);
            addError(e);
        }
    }


    /**
     * Resize the table if needed
     */
    if (st->count + 1 >= st->capacity) { 
        __resizeSymbolTable(st);
    }

    /**
     * Find the index in which we are gonna insert the symbol s 
     * By default, is the last one, but there could be sorting if the SymbolTable is linked to a function
     */
    int index_of_s = st->count;


    // If the symbol table belongs to a function, then we sort to reduce padding. 
    // Don't sort if it belongs to an object or class. Sort IF belongs to a function
    if (st->kind == KIND_FUNCTION_ST)
    {
        //printf("Sorting the ST\n");
        //Order the table in correct order, big sizes first
        for (int i = 0; i < st->count; i++)
        {
            if (s->smi->size >= st->symbols[i]->smi->size)
            {
                index_of_s = i;
                // Add one to all the consecutive symbols' index
                // and shift them one down
                for (int j = st->count; j > i; j--)
                {
                    st->symbols[j] = st->symbols[j - 1];
                    st->symbols[j]->smi->index++;
                }

                break;
            }
        }
    }

    /**
     * Add it to the table
     */
    st->count++;
    s->smi->index = index_of_s;
    st->symbols[s->smi->index] = s;


    /**
     * Recompute the offsets of all the symbols down the table
     */
    for (int i = s->smi->index; i < st->count; i++){

        st->symbols[i]->smi->offset =  (i == 0) ? 0 : st->symbols[i - 1]->smi->offset + st->symbols[i-1]->smi->size;

        // Align to an offset that is multiple of the variable size
        // printf("Aligning the symbol: %s\n", st->symbols[i]->smi->identifier );
        st->symbols[i]->smi->offset = align_to(st->symbols[i]->smi->offset, st->symbols[i]->smi->type);

        //printf("Finished aligning the symbol\n");

    }


    /**
     * Add the size of the symbol to the total weight of the table
     */
    st->weight += s->smi->size;


    /**
     * Compute the padding
     */
    st->padding = 0;
    for (int i = 1; i < st->count; i++)
    {
        st->padding += st->symbols[i]->smi->offset - (st->symbols[i - 1]->smi->offset + st->symbols[i - 1]->smi->size);
    }

}

void absorbSymbolsIntoTable(SymbolTable *absorber_st, SymbolTable *absorbee_st)
{
    for (int i = 0; i < absorbee_st->count; i++){
        addSymbolToTable(absorbee_st->symbols[i], absorber_st);
    }
}

int doSymbolsClash(Symbol *s1, Symbol *s2)
{
    /**
     * If they have a different scope, they don't clash
     */
    if (s1->smi->scope != s2->smi->scope){
        return 0;
    }

    /**
     * Same scope. If they have a different name, they do not clash
     */
    if (strcmp(s1->smi->identifier, s2->smi->identifier) != 0){
        return 0;
    }

    /**
     * At this point they have the same name and same scope.
     * If they are globals, only allowed if one of them is external and it hasnt been alloc'ed
     * 
     */
    if (s1->symbol_kind == SYMBOL_GLOBAL && s2->symbol_kind == SYMBOL_GLOBAL){
        if ((s1->s_global.global_kind == GLOBAL_EXTERN && !s1->s_global.symbolExternVar.is_allocated) && s2->s_global.global_kind != GLOBAL_EXTERN){
            s1->s_global.symbolExternVar.is_allocated = 1;
            return 0; // they dont clash
        }
        if ((s2->s_global.global_kind == GLOBAL_EXTERN && !s2->s_global.symbolExternVar.is_allocated) && s1->s_global.global_kind != GLOBAL_EXTERN){
            s2->s_global.symbolExternVar.is_allocated = 1;
            return 0;
        }
        return 1;
    }

    // * If they are both parameters, not allowed
    if (s1->s_local.local_kind == LOCAL_PARAMETER && s2->s_local.local_kind == LOCAL_PARAMETER){
        return 1;
    }

    /**
     * At this point, one of them is a parameter or both are local vars
     */
    if (s1->s_local.local_kind != s2->s_local.local_kind){
        return 1;
    }

    /**
     * At this point, we have the same name, same scope, and they are both local vars
     * If different context definition, we are fine
     */
    if (s1->smi->def_ctx.ctx_type != s2->smi->def_ctx.ctx_type){
        return 0;
    }

    /**
     * If different context id, we are fine
     */
    if (s1->smi->def_ctx.block_id != s2->smi->def_ctx.block_id){
        return 0;
    }

    /**
     * If we got here, they have the same name, same scope, defined in the same context and in the same block id
     * Not allowed
     */
    return 1;
}


/**
 * ===============================================================================
 * END SYMBOL OPERATIONS     
 * ===============================================================================   
 */


/**
 * ===============================================================================
 * FUNCTION OPERATIONS     
 * ===============================================================================   
 */

FunctionCandidates *newFunctionCandidates()
{
    FunctionCandidates *fc = malloc(sizeof(FunctionCandidates));
    fc->capacity = INIT_FUNC_CANDIDATES_CAPACITY;
    fc->count = 0;
    fc->func_candidates = malloc(sizeof(Function *) * fc->capacity);

    return fc;
}

void __resizeFunctionCandidates(FunctionCandidates *fc)
{
    fc->capacity *= FUNC_CANDIDATES_RESIZE_FACTOR;
    fc->func_candidates = realloc(fc->func_candidates, fc->capacity * sizeof(Function *));
}

void addCandidate(FunctionCandidates *fc, Function *f)
{
    if (fc->count + 1 >= fc->capacity){
        __resizeFunctionCandidates(fc);
    }

    fc->func_candidates[fc->count++] = f;
}

FunctionTable *__allocateFunctionTable()
{
    FunctionTable *ft = malloc(sizeof(FunctionTable));
    ft->capacity = FUNCTIONTABLE_INIT_CAPACITY;
    ft->functions = malloc(FUNCTIONTABLE_INIT_CAPACITY * sizeof(Function *));

    return ft;
}

void __resizeFunctionTable(FunctionTable *ft)
{
    ft->capacity = FUNCTIONTABLE_RESIZE_FACTOR * ft->capacity;
    ft->functions = realloc(ft->functions, ft->capacity * sizeof(Function *));
}

FunctionTable *makeFunctionTable(FunctionTable *parentFT)
{
    FunctionTable *ft = __allocateFunctionTable();
    ft->capacity = FUNCTIONTABLE_INIT_CAPACITY;
    ft->count = 0;
    ft->parentFT = parentFT;
    ft->scope = getCurrentScope();

    return ft;
}

void __initGlobalFunctionTable()
{
    gb_functionTable = makeFunctionTable(NULL);
}

FunctionMetaInfo *__allocateFunctionMetaInfo()
{
    FunctionMetaInfo *fmi = malloc(sizeof(FunctionMetaInfo));
    fmi->local_functions = __allocateFunctionTable();
    fmi->local_symbols = __allocateSymbolTable();
    fmi->reference_labels = __allocateLabel();

    return fmi;
}

Function *__allocateFunction()
{
    Function *f = malloc(sizeof(Function));
    f->fmi = __allocateFunctionMetaInfo();
    
    return f;
}

int checkForVarargs(Parameter **parameters, int param_count)
{
    for (int i = 0; i < param_count; i++){
        if (parameters[i]->ptype == P_VARARGS){
            return 1;
        }
    }
    return 0;
}

FunctionMetaInfo *makeFunctionMetaInfo(char *name, int param_count, Parameter **parameters, Type *rt_type, FunctionTable *parentFT, SymbolTable *parentST)
{
    FunctionMetaInfo *fmi = __allocateFunctionMetaInfo();
    fmi->scope = getCurrentScope();
    fmi->name = name;
    fmi->has_varargs = checkForVarargs(parameters, param_count);
    fmi->param_count = param_count;
    fmi->parameters = parameters;
    fmi->rt_type = rt_type;
    fmi->def_line_number = tracker.current_line;
    fmi->def_src_file = tracker.current_src_file;
    fmi->local_functions = makeFunctionTable(parentFT);
    fmi->local_symbols = makeSymbolTable(parentST, KIND_FUNCTION_ST);

    //printf("Made FMI with name: %s, scope = %i\n", fmi->name, fmi->scope);

    return fmi;
}

Function *makeMethodFunction(char *name, int param_count, Parameter **parameters, Type *rt_type, Class *owner_class, FunctionTable *parentFT, SymbolTable *parentST)
{
    Function *f = __allocateFunction();
    f->fmi = makeFunctionMetaInfo(name, param_count, parameters, rt_type, parentFT, parentST);
    f->kind = FUNCTION_METHOD;
    f->method.class_owner = owner_class;

    return f;

}

Function *makeSingleFunction(char *name, int param_count, Parameter **parameters, Type *rt_type, FunctionTable *parentFT, SymbolTable *parentST, int is_forward)
{
    Function *f = __allocateFunction();
    f->fmi = makeFunctionMetaInfo(name, param_count, parameters, rt_type, parentFT, parentST);
    f->kind = FUNCTION_SINGLE;
    f->single.is_forward = is_forward;
    return f;
}

void addFunctionToTable(Function *f, FunctionTable *ft)
{
    for (int i = 0; i < ft->count; i++){
        if (doFunctionsClash(ft->functions[i], f)){
            
            // We found the definition of forwarded ft->functions[i], swap it with the actual def: f
            if (ft->functions[i]->kind == FUNCTION_SINGLE && ft->functions[i]->single.is_forward == 1){
                //printf("Found the def of forward func: %s, now function staying at table = %p\n", ft->functions[i]->fmi->name, f);
                ft->functions[i] = f;
                return;
            }
            if (f->kind == FUNCTION_SINGLE && f->single.is_forward == 1){
                // There is already a completely defined function matching this forward decl, just ignore
                return;
            }

            if (ft->functions[i]->fmi->has_varargs || f->fmi->has_varargs){
                Error *e = invoke(VARARG_FUNC_MUST_HAVE_UNIQUE_NAME);
                appendMessageToError(e, fetchErrorMsgBasedOnErrorID(e->error_id));
                appendMessageToError(e, "Provided function name: '%s', is also the name of a function defined in file: '%s', in L = %i.", 
                                     f->fmi->name, ft->functions[i]->fmi->def_src_file, ft->functions[i]->fmi->def_line_number);
                addError(e);
            }
            else{
                Error *e = invoke(REDEFINITION_OF_FUNCTION);
                appendMessageToError(e, fetchErrorMsgBasedOnErrorID(e->error_id), f->fmi->name);
                appendMessageToError(e, "Previous definition in file: '%s', in L = %i.", ft->functions[i]->fmi->def_src_file, ft->functions[i]->fmi->def_line_number);
                addError(e);
            }
        }
    }

    if (ft->count + 1 >= ft->capacity) { 
        __resizeFunctionTable(ft); 
    }

    ft->functions[ft->count++] = f;
}

int doFunctionsClash(Function *f, Function *g)
{
    if (f->kind != g->kind){
        return 0;
    }

    if (strcmp(f->fmi->name, g->fmi->name) != 0){
        return 0;
    }

    // Same name, at least one has varargs, not allowed
    if (f->fmi->has_varargs || g->fmi->has_varargs){
        return 1;
    }

    if (f->fmi->param_count != g->fmi->param_count){
        return 0;
    }

    int allParamsMatch = 1;
    for (int i = 0; i < f->fmi->param_count; i++){
        if (!areTypesEqual(f->fmi->parameters[i]->type, g->fmi->parameters[i]->type)){
            allParamsMatch = 0;
            break;
        }
    }
    return allParamsMatch;
}

Function *selectBestFunctionCandidateOf(Function *f1, Function *f2, Type **param_types, int param_count)
{
    if (f1 == NULL){
        return f2;
    }
    if (f2 == NULL){
        return f1;
    }

    int f1_wins = 0;
    int f2_wins = 0;

    for (int i = 0; i < param_count; i++){
        Type *t1 = f1->fmi->parameters[i]->type;
        Type *t2 = f2->fmi->parameters[i]->type;

        int t1_equal = areTypesEqual(t1, param_types[i]);
        int t2_equal = areTypesEqual(t2, param_types[i]);

        // Could be built-in
        if (t1_equal || t2_equal){
            if (t1_equal){
                f1_wins++;
            }
            if (t2_equal){
                f2_wins++;
            }
        }
        
        else if (isTypeOfKind(param_types[i], TYPE_UDT)){
            if (isTypeOfKind(param_types[i], TYPE_PTR)){
                t1 = getFinalPointedType(f1->fmi->parameters[i]->type);
                t2 = getFinalPointedType(f2->fmi->parameters[i]->type);
            }
            int d1 = generationLevelsBetweenTypes(t1, param_types[i]);
            int d2 = generationLevelsBetweenTypes(t2, param_types[i]);

            if (d1 < d2){
                f1_wins++;
            }

            else if (d1 > d2){
                f2_wins++;
            }
        }
        
    }

    if (f1_wins > f2_wins){
        return f1;
    }
    if (f2_wins == f1_wins){
        Error *e = invoke(AMBIGUITY_DURING_FUNCCALL);
        appendMessageToError(e, fetchErrorMsgBasedOnErrorID(e->error_id), f1->fmi->name);
        appendMessageToError(e, "Provided parameters types are: \n(");
        for (int i = 0; i < param_count; i++){
            appendMessageToError(e, "%s", typeToString(param_types[i]));
            if (i < param_count - 1){
                appendMessageToError(e, ",");
            }
        }
        appendMessageToError(e, ")\n");
        appendMessageToError(e, "Function defined in file = '%s', in L = %i, can accept such parameter types.\n", f1->fmi->def_src_file, f1->fmi->def_line_number);
        appendMessageToError(e, "Function defined in file = '%s', in L = %i, can accept such parameter types.\n", f2->fmi->def_src_file, f2->fmi->def_line_number);
        addError(e);
    }
    return f2;

}

Function *lookUpFunction(FunctionTable *ft, int scope, char *identifier, Type **param_types, int param_count)
{


    FunctionCandidates *fc = newFunctionCandidates();

    FunctionTable *current_ft = ft;
    while (current_ft != NULL){
        // printf("Looking in the following ft for '%s'\n", identifier);
        // print_functionTable(current_ft);
        int checking_scope = scope;
        while (checking_scope >= 0)
        {
            for (int i = 0; i < current_ft->count; i++){
                /**
                 * No need to check kind, foo(ptr<Class>) will never retrieve a match since we are starting to look in a different function table
                 */

                if (current_ft->functions[i]->fmi->scope != checking_scope){
                    continue;
                }
                // printf("Checking in scope: %i, with params: %i. Candidate: %s, scope: %i\n", checking_scope, param_count, current_ft->functions[i]->fmi->name, current_ft->functions[i]->fmi->scope);

                //printf("Giot this far, candidate = %s, identifier = %s\n", current_ft->functions[i]->fmi->name, identifier);

                if (strcmp(current_ft->functions[i]->fmi->name, identifier) != 0){
                    continue;
                }

                /** If function has varargs, return this one  [we return the f in the same checking scope, with the same name, with varargs] */
                if (current_ft->functions[i]->fmi->has_varargs){
                    return current_ft->functions[i];
                } 

                if (current_ft->functions[i]->fmi->param_count != param_count){
                    continue;
                }

                // printf("Got a candidate with same params, scope, and name: '%s'\n", current_ft->functions[i]->fmi->name);

                int allParamsMatch = 1;
                for (int k = 0; k < param_count; k++){
                    if (!typesMatchForFunctionCall(current_ft->functions[i]->fmi->parameters[k]->type, param_types[k])){
                        allParamsMatch = 0;
                        break;
                    }
                }

                if (allParamsMatch){
                    addCandidate(fc, current_ft->functions[i]);
                    //return current_ft->functions[i];
                }

            }
            checking_scope--;
        }
        
        current_ft = current_ft->parentFT;
    }

    /** We have a list of candidates now */
    Function *bestMatch = fc->func_candidates[0];
    for (int i = 1; i < fc->count; i++){
        bestMatch = selectBestFunctionCandidateOf(bestMatch, fc->func_candidates[i], param_types, param_count);
    }


    return bestMatch;
}


void populateFunctionLabel(Function *f)
{
    f->fmi->reference_labels = makeCompoundLabel();
    char *buffer = malloc(256);
    memset(buffer, 0, 256);
    char *ptr = buffer;
    for (int i = 0; i < f->fmi->param_count; i++){

        if (f->fmi->parameters[i]->ptype == P_VARARGS){
            ptr += sprintf(ptr, "_%s", "vargs");
        }
        else{
            ptr += sprintf(ptr, "_%s", shortForType(f->fmi->parameters[i]->type));
        }
    }

    Tag *start_tag = makeTag("_%s%s", f->fmi->name, buffer);
    Tag *end_tag = makeTag("ret_from%s", start_tag->tag_name);

    setStartTagToCompoundLabel(f->fmi->reference_labels, start_tag);
    setEndTagToCompoundLabel(f->fmi->reference_labels, end_tag);

    //printf("Start label for func: %s, defined at L = %i, is: %s \n", f->fmi->name, f->fmi->def_line_number, f->fmi->reference_labels->compound_label.start_tag->tag_name);
}


/**
 * ===============================================================================
 * END FUNCTION OPERATIONS     
 * ===============================================================================   
 */

/**
 * ===============================================================================
 * OBJECT OPERATIONS     
 * ===============================================================================   
 */

Object *__allocateObject()
{
    Object *o = malloc(sizeof(Object));
    o->local_symbols = __allocateSymbolTable();
    return o;
}

ObjectTable *__allocateObjectTable()
{
    ObjectTable *ot = malloc(sizeof(ObjectTable));
    ot->capacity = OBJECTTABLE_INIT_CAPACITY;
    ot->objects = malloc(sizeof(Object *) * OBJECTTABLE_INIT_CAPACITY);

    return ot;
}


void __resizeObjectTable(ObjectTable *ot)
{
    ot->capacity = OBJECTTABLE_RESIZE_FACTOR * ot->capacity;
    ot->objects = realloc(ot->objects, ot->capacity * sizeof(Object *));
}


ObjectTable *makeObjectTable()
{
    ObjectTable *ot = __allocateObjectTable();
    ot->count = 0;

    return ot;
}

void __initGlobalObjectTable()
{
    gb_objectTable = makeObjectTable();
}

Object *makeObject(char *objectName, Object *parentObject, int is_forward, Type *object_type)
{
    // printf("MADE AN OBJECT TYPE AT %p\n", object_type);
    Object *o = __allocateObject();
    o->identifier = objectName;
    o->parent = parentObject;
    o->def_line_number = tracker.current_line;
    o->def_src_file = tracker.current_src_file;
    o->is_forward = is_forward;
    o->type = object_type;
    // Mark this udt as object
    o->type->udt.udt_kind = UDT_OBJECT;
    object_type->representation = updateTypeRepresentation(object_type);
    gb_typeTable->undef_count--;

    if (parentObject != NULL){
        o->local_symbols = makeSymbolTable(parentObject->local_symbols, KIND_OBJECT_ST);
    }
    else{
        o->local_symbols = makeSymbolTable(NULL, KIND_OBJECT_ST);
    }


    populateUDTInfo(UDT_OBJECT, o->identifier);

    return o;
}

Object *lookUpObject(ObjectTable *ot, char *name)
{
    for (int i = 0; i < ot->count; i++)
    {
        if (strcmp(ot->objects[i]->identifier, name) == 0){
            return ot->objects[i];
        }
    }

    return NULL;    
}

void addObjectToTable(ObjectTable *ot, Object *o)
{
    int gotta_add = checkConflictingUDTs(o->identifier, UDT_OBJECT, (void *)o);

    if (!gotta_add){ 
        return;
    }

    if (ot->count + 1 >= ot->capacity){
        __resizeObjectTable(ot);
    }

    ot->objects[ot->count++] = o;
}

/**
 * ===============================================================================
 * END OBJECT OPERATIONS     
 * ===============================================================================   
 */

/**
 * ===============================================================================
 * CLASS OPERATIONS     
 * ===============================================================================   
 */

Class *__allocateClass()
{
    Class *c = malloc(sizeof(Class));
    c->local_functions = __allocateFunctionTable();
    c->local_symbols = __allocateSymbolTable();

    return c;
}

void __resizeClassTable(ClassTable *ct)
{
    ct->capacity = CLASSTABLE_RESIZE_FACTOR * ct->capacity;
    ct->classes = realloc(ct->classes, ct->capacity * sizeof(Class *));
}

ClassTable *__allocateClassTable()
{
    ClassTable *ct = malloc(sizeof(ClassTable));
    ct->capacity = CLASSTABLE_INIT_CAPACITY;
    ct->classes = malloc(CLASSTABLE_INIT_CAPACITY * sizeof(ClassTable));

    return ct;
}

ClassTable *makeClassTable()
{
    ClassTable *ct = __allocateClassTable();
    ct->count = 0;
    return ct;
}

void __initGlobalClassTable()
{
    gb_classTable = makeClassTable();
}

Class *makeClass(char *name, Class *parentClass, int is_forward, Type *class_type)
{
    Class *c = __allocateClass();
    c->identifier = name;
    c->parent = parentClass;
    c->def_line_number = tracker.current_line;
    c->def_src_file = tracker.current_src_file;
    c->is_forward = is_forward;
    c->type = class_type;
    // Mark this udt as class
    c->type->udt.udt_kind = UDT_CLASS;
    gb_typeTable->undef_count--;


    if (parentClass != NULL){
        c->local_functions = makeFunctionTable(parentClass->local_functions);
        c->local_symbols = makeSymbolTable(parentClass->local_symbols, KIND_CLASS_ST);
    }
    else{
        c->local_functions = makeFunctionTable(gb_functionTable);
        c->local_symbols = makeSymbolTable(gb_symbolTable, KIND_CLASS_ST);
    }
    //printTypeTable();

    populateUDTInfo(UDT_CLASS, c->identifier);
    
    // printTypeTable();

    return c;
}



Class *lookUpClass(ClassTable *ct, char *name)
{
    for (int i = 0; i < ct->count; i++)
    {
        if (strcmp(ct->classes[i]->identifier, name) == 0){
            return ct->classes[i];
        }
    }

    return NULL;    
}



void addClassToTable(ClassTable *ct, Class *c)
{

    int gotta_add = checkConflictingUDTs(c->identifier, UDT_CLASS, (void *)c);

    if (!gotta_add){
        return;
    }

    if (ct->count + 1 >= ct->capacity){
        __resizeClassTable(ct);
    }

    ct->classes[ct->count++] = c;
}

/**
 * ===============================================================================
 * END CLASS OPERATIONS     
 * ===============================================================================   
 */



/**
 * ===============================================================================
 * ENUM OPERATIONS     
 * ===============================================================================   
 */



Enum *__allocateEnum()
{
    Enum *e = malloc(sizeof(Enum));
    return e;
}

EnumTable *__allocateEnumTable()
{
    EnumTable *et = malloc(sizeof(EnumTable));
    et->capacity = ENUMTABLE_INIT_CAPACITY;
    et->enums = malloc(ENUMTABLE_INIT_CAPACITY * sizeof(Enum *));
    
    return et;
}

EnumTable *makeEnumTable()
{
    EnumTable *et = __allocateEnumTable();
    et->count = 0;

    return et;
}

void __initGlobalEnumTable()
{
    gb_enumTable = makeEnumTable();
}

void __resizeEnumTable(EnumTable *et)
{
    et->capacity = ENUMTABLE_RESIZE_FACTOR * et->capacity;
    et->enums = realloc(et->enums, et->capacity * sizeof(Enum *));
}

Enum *makeEnum(char *name, int is_forward, Type *enum_type, Type *base_type)
{
    // printf("Before making enum:\n");
    // printTypeTable();
    Enum *e = __allocateEnum();
    e->identifier = name;
    e->def_line_number = tracker.current_line;
    e->def_src_file = tracker.current_src_file;
    e->is_forward = is_forward;
    e->type = enum_type;
    // Mark this udt as enum
    e->type->udt.udt_kind = UDT_ENUM;
    gb_typeTable->undef_count--;

    e->base_type = base_type;
    // printf("After making enum\n");
    // printTypeTable();
    populateUDTInfo(UDT_ENUM, e->identifier);

    return e;
}


Enum *lookUpEnum(EnumTable *et, char *name)
{
    for (int i = 0; i < et->count; i++)
    {
        if (strcmp(et->enums[i]->identifier, name) == 0)
        {
            return et->enums[i];
        }
    }

    return NULL;    
}


void addEnumToTable(EnumTable *et, Enum *e)
{
    int gotta_add = checkConflictingUDTs(e->identifier, UDT_ENUM, (void *)e);

    if (!gotta_add){
        return;
    }

    //Add
    if (et->count + 1 >= et->capacity) { 
        __resizeEnumTable(et); 
    }

    et->enums[et->count++] = e;
}

/**
 * ===============================================================================
 * END ENUM OPERATIONS     
 * ===============================================================================   
 */



/**
 * ===============================================================================
 * ANALYSIS VALUE OPERATIONS     
 * ===============================================================================   
 */


Value *__allocateValue()
{
    Value *v = malloc(sizeof(Value));
    v->type = __allocateType();
    v->size = 0;
    return v;
}

Value *makeSpecificValue(Type *type)
{
    if (type == NULL){
        fprintf(stderr, "Cnan't make a specific value out of a NULL type\n");
        exit(1);
    }

    Value *v = __allocateValue();
    v->type = type;
    if (v->type->kind == TYPE_VOID){
        v->size = 0;
    }
    else{
        v->size = calculateSizeOfType(type);
    }

    return v;
}

/**
 * Useful for nodes that return nothing like an assignment
 */
Value *makeVoidValue()
{
    Value *v = __allocateValue();
    v->type->kind = TYPE_VOID;
    v->size = 0;
    return v;
}



/**
 * ===============================================================================
 * END ANALYSIS VALUE OPERATIONS     
 * ===============================================================================   
 */

void resetContext()
{
    context.current_scope = 0;
    context.stack_index = 0;
    context.if_block_id = 0;
    context.enum_block_id = 0;
    context.object_block_id = 0;
    context.union_block_id = 0;
    context.class_block_id = 0;
    context.elseif_block_id = 0;
    context.else_block_id = 0;
    context.for_block_id = 0;
    context.foreach_block_id = 0;
    context.while_block_id = 0;
    context.ext_funct_id = 0;
    context.function_block_id = 0;
    context.skip_right_id = 0;
    context.current_class = NULL;
    context.current_object = NULL;
    //context.ctx_type_stack[0] = CTX_NONE;
}





//Check if parent is a loop ---> We could have a for loop and then a skip inside an if, 
//thats allowed 
int current_ctx_is_inside_loop()
{
    int scope_tracker = context.current_scope;
    while (scope_tracker > 1)
    {
        switch (context.ctx_block_stack[scope_tracker].ctx_type)
        {   
            case CTX_NONE: return 0;
            case CTX_FUNCTION: return 0;
            case CTX_FOR: return 1;
            case CTX_FOREACH: return 1;
            case CTX_WHILE: return 1;
            default: scope_tracker--;
        }
    }

    return 0;

}


void printStringTable()
{
    printf("============================== STRING TABLE =========================== \n");
    for (int i = 0; i < gb_stringTable->count; i++)
    {
        printf("%i: (label = %s, str = %s)\n", i, gb_stringTable->labels[i]->tag.tag_name, gb_stringTable->saved_strings[i]);
    }
    printf("============================== END STRING TABLE =========================== \n");

}


int getAlignmentSize(Type *type)
{
    int size = 0;
    switch (type->kind)
    {
        case TYPE_ARRAY:{
            size = getAlignmentSize(type->arrayType.element_type);
            break;
        }
        case TYPE_UDT:{
            int max_align = 1;
            switch(type->udt.udt_kind){
                case UDT_CLASS:{
                    Class *c = lookUpClass(gb_classTable, type->udt.name);
                    
                    if (c->is_forward){
                        Error *e = invoke(TYPE_INCOMPLETE);
                        appendMessageToError(e, fetchErrorMsgBasedOnErrorID(e->error_id), typeToString(type));
                        appendMessageToError(e, "Can't calculate alignment size of incomplete type.");
                        appendMessageToError(e, "Consider using a pointer or defining the type before usage.");
                        addError(e);
                        break;
                    }

                    for (int i = 0; i < c->local_symbols->count; i++){
                        int align_size = getAlignmentSize(c->local_symbols->symbols[i]->smi->type);
                        if (align_size > max_align) { max_align = align_size; }
                    }
                    break;
                }
                case UDT_OBJECT:{

                    //printf("Getting alignment size for object: %s\n", typeToString(type));
                    Object *o = lookUpObject(gb_objectTable, type->udt.name);
                    if (o->is_forward){
                        Error *e = invoke(TYPE_INCOMPLETE);
                        appendMessageToError(e, fetchErrorMsgBasedOnErrorID(e->error_id), typeToString(type));
                        appendMessageToError(e, "Can't calculate alignment size of incomplete type.");
                        appendMessageToError(e, "Consider using a pointer or defining the type before usage.");
                        addError(e);
                        break;
                    }
                    for (int i = 0; i < o->local_symbols->count; i++){
                        int align_size = getAlignmentSize(o->local_symbols->symbols[i]->smi->type);
                        if (align_size > max_align) { max_align = align_size; }
                    }
                    break;
                }
                case UDT_ENUM:{
                    Enum *e = lookUpEnum(gb_enumTable, type->udt.name);
                    max_align = getAlignmentSize(e->base_type);
                    break;
                }
                case UDT_UNKNOWN_YET:{
                    fprintf(stderr, "Cant get alignment size for unknown udt\n");
                    exit(1);
                }
            }
            size = max_align;
            break;
        }
        // Primitive types
        default:{
            size = calculateSizeOfType(type);
            break;
        }
    }

    //printf("Turns out to be: %i\n", size);
    return size;
}

int align_to(int offset, Type *type)
{
    int size = getAlignmentSize(type);
    return (offset + (size - 1) - (size + offset - 1) % size);
}





//Populate the tables and analyze whether expressions are valid
Value *analyze(ASTNode *program, SymbolTable *current_st, FunctionTable *current_ft)
{
    if (program == NULL || program->node_type == NODE_NULL) { 
        Value *v = makeVoidValue(); 
        return v; 
    }
    
    update_tracker(program);
    // printf("L = %i. Analyzing node with type: %s (%hhu)\n", tracker.current_line, astTypeToStr(program), program->node_type);

    switch (program->node_type)
    {

        case NODE_USE_DIRECTIVE:
        {
            int line_to_resume = tracker.current_line;
            char *file_to_resume = tracker.current_src_file;

            tracker.current_src_file = program->use_node.filepath;

            analyze(program->use_node.program, current_st, current_ft);

            tracker.current_line = line_to_resume;
            tracker.current_src_file = file_to_resume;

            Value *result = makeVoidValue();
            return result;
        }
        
        case NODE_BLOCK:
        {
            Value *result = makeVoidValue();
            //Default init value if block is empty
            for (int i = 0; i < program->block_node.statement_count; i++){
               result = analyze(program->block_node.statements[i], current_st, current_ft);
            }

            // Statements return void unless its a return node
            // This fix makes up for:
            // func type foo() {  3; } --> compiler will complain saying that we return uint8 instead
            if (program->block_node.statement_count > 0 && 
                program->block_node.statements[program->block_node.statement_count - 1]->node_type != NODE_RETURN &&
                program->block_node.statements[program->block_node.statement_count - 1]->is_stmt){
                result->type = makeVoidType();
                result->size = 0;
            }
            return result;
        }

        case NODE_NULL:
        {
            Value *result = makeVoidValue();
            return result;
        }
        
        case NODE_EXTERN_FUNC_DEF:
        {
            if (getCurrentContextType() != CTX_NONE){
                Error *e = invoke(EXTERN_FUNC_DEF_ONLY_ALLOWED_GLOBALLY);
                appendMessageToError(e, fetchErrorMsgBasedOnErrorID(e->error_id), program->extern_func_def_node.func_name);
                addError(e);
            }

            //Increment scope, we are inside the function, push the context 
             //Create a function. Create its local symbol table and local function table
            Function *f = makeSingleFunction(program->extern_func_def_node.func_name, program->extern_func_def_node.params_count, 
                                             program->extern_func_def_node.params, program->extern_func_def_node.return_type, current_ft, current_st, 0);
            

            populateFunctionLabel(f);   
            addFunctionToTable(f, current_ft);
            
            program->extern_func_def_node.function = f;



            //First populate the function's local symbol table with the parameters and possible functions
            for (int i = 0; i < program->extern_func_def_node.params_count; i++){
                Symbol *parameter = makeParameterSymbol(program->extern_func_def_node.params[i]->name, i, program->extern_func_def_node.params[i]->type);
                addSymbolToTable(parameter, f->fmi->local_symbols);
            }
            

            //Statements that return nothing
            Value *none = makeVoidValue();
            return none;
        }

        case NODE_FUNC_DEF:
        {
            
            //Increment scope, we are inside the function, push the context 

            Class *c = getCurrentClass();
            Function *f = NULL;

            if (c == NULL){
                f = makeSingleFunction(program->funcdef_node.func_name, program->funcdef_node.params_count,
                                       program->funcdef_node.params, program->funcdef_node.return_type, current_ft, current_st, program->funcdef_node.is_forward);
            }
            // Is a method
            else{
                Type *c_type = findUDTInTable(c->identifier);
                Type *this_type = createPointerTypeFromInfo(c_type);
                Symbol *this = makeParameterSymbol("this", 0, this_type);

                insertNewParameter(this, program->funcdef_node.params, program->funcdef_node.params_count);
                program->funcdef_node.params_count++;


                f = makeMethodFunction(program->funcdef_node.func_name, program->funcdef_node.params_count, 
                                       program->funcdef_node.params, program->funcdef_node.return_type, c, current_ft, current_st);
            }


            /**
             * Analyze the return type, cannot be array or bigger than 8 bytes
             */

            if (isTypeOfKind(f->fmi->rt_type, TYPE_ARRAY)){
                Error *e = invoke(RETURN_TYPE_IS_ARRAY);
                appendMessageToError(e, fetchErrorMsgBasedOnErrorID(e->error_id), f->fmi->name);
                addError(e);
            }
            else if (!isTypeOfKind(f->fmi->rt_type, TYPE_VOID) && calculateSizeOfType(f->fmi->rt_type) > 8){  
                Error *e = invoke(RETURN_TYPE_BIGGER_8B);
                appendMessageToError(e, fetchErrorMsgBasedOnErrorID(e->error_id), f->fmi->name, typeToString(f->fmi->rt_type));
                addError(e);             
            }

            populateFunctionLabel(f);
            addFunctionToTable(f, current_ft);

            if (program->funcdef_node.is_forward == 1){
                Value *none = makeVoidValue();
                return none;
            }

            // Push context now so global functions have scope 0, and so on
            pushContext(CTX_FUNCTION);

            program->funcdef_node.function = f;


            // Add parameters to symbol table
            for (int i = 0; i < program->funcdef_node.params_count; i++){

                if (program->funcdef_node.params[i]->ptype == P_VARARGS){
                    if (i != program->funcdef_node.params_count - 1){
                        // Varargs must be the last parameter
                        Error *e = invoke(VARARG_MUST_BE_LAST_PARAM);
                        appendMessageToError(e, fetchErrorMsgBasedOnErrorID(e->error_id), f->fmi->name);
                        addError(e);
                    }
                    else{
                        continue;
                    }
                }

                Symbol *p = makeParameterSymbol(program->funcdef_node.params[i]->name, i, program->funcdef_node.params[i]->type);

                /**
                 * If parameter == array || parameter > 8 bytes ===> arrays are not allowed, pass by reference
                 */
                if (isTypeOfKind(p->smi->type, TYPE_ARRAY)){
                    Error *e = invoke(PARAM_TYPE_IS_ARRAY);
                    appendMessageToError(e, fetchErrorMsgBasedOnErrorID(e->error_id), f->fmi->name, p->smi->identifier);
                    addError(e);
                }
                else if (p->smi->size > 8){
                    Error *e = invoke(PARAM_TYPE_BIGGER_8B);
                    appendMessageToError(e, fetchErrorMsgBasedOnErrorID(e->error_id), f->fmi->name, p->smi->identifier, typeToString(p->smi->type));
                    addError(e);
                }

                if (!doesTypeExist(p->smi->type)){
                    printf("Here\n");
                    Error *e = invoke(TYPE_UNDEFINED);
                    appendMessageToError(e, fetchErrorMsgBasedOnErrorID(e->error_id), typeToString(p->smi->type));
                    addError(e);
                    printf("There\n");
                }
                
                p->smi->type->representation = updateTypeRepresentation(p->smi->type);
                addSymbolToTable(p, f->fmi->local_symbols);
            }


            //Analyze the body of the function
            Value *return_value = analyze(program->funcdef_node.body, f->fmi->local_symbols, f->fmi->local_functions);


            if (!isTypeAssignableToDeclared(f->fmi->rt_type, return_value->type)){
                Error *e = invoke(ACTUAL_RETTYPE_CANT_BE_ASSIGNED_TO_DECLARED_RETTYPE);
                appendMessageToError(e, fetchErrorMsgBasedOnErrorID(e->error_id), f->fmi->name, typeToString(f->fmi->rt_type), typeToString(return_value->type));
                addError(e);
            }

            //We are outside the function, pop the context and scope 
            popContext();
            //Statements that return nothing
            Value *none = makeVoidValue();
            return none;
        }

        case NODE_CONTINUE:
        {
            // printf("Inside continue ...\n");
            //Continue can only be used within a loop 
            if (!current_ctx_is_inside_loop()){
                Error *e = invoke(CONTINUE_NOT_IN_LOOP);
                appendMessageToError(e, fetchErrorMsgBasedOnErrorID(e->error_id));
                addError(e);
            }

            Label *l = getCurrentCtxLabel();
            Tag *t = NULL;
            if (l->compound_label.intermediate_tags == NULL || l->compound_label.intermediate_tags[0] == NULL){
                t = l->compound_label.end_tag;
            }
            else{
                t = l->compound_label.intermediate_tags[0];
            }
            // printf("Getting tag for continue = %s\n", t->tag_name);
            // printf("Got here\n");
            program->continue_node.tag = t;

            Value *none = makeVoidValue();
            return none;
        }

        case NODE_BREAK:{
            // Break can only be used within a loop
            if (!current_ctx_is_inside_loop()){
                Error *e = invoke(BREAK_NOT_IN_LOOP);
                appendMessageToError(e, fetchErrorMsgBasedOnErrorID(e->error_id));
                addError(e);
            }

            Label *l = getCurrentCtxLabel();
            // printf("Getting tag for break = %s\n", l->compound_label.end_tag->tag_name);

            program->break_node.tag = l->compound_label.end_tag;
            Value *none = makeVoidValue();
            return none;
        }

        case NODE_WHILE:
        {
            program->while_node.label = makeCompoundLabel();
            Tag *while_start = makeTag("while_loop_%i", getCurrentContextBlockId(CTX_WHILE));
            setStartTagToCompoundLabel(program->while_node.label, while_start);
            Tag *while_end = makeTag("end_%s", while_start->tag_name);
            setEndTagToCompoundLabel(program->while_node.label, while_end);

            //Analyze the condition
            analyze(program->while_node.condition_expr, current_st, current_ft);

            //Increment current scope, we are inside the while
            pushContext(CTX_WHILE); 
            pushCtxLabel(program->while_node.label);
            //Analyze the body
            analyze(program->while_node.body, current_st, current_ft);
            //analyze(program->while_node.body, current_st, current_ft);

            //Decrement current scope, while loop is finished
            popContext(); 
            popCtxLabel();

            //Statements that return nothing
            Value *result = makeVoidValue();
            return result;

        }

        case NODE_IF:
        {

            program->if_node.reference_label = makeCompoundLabel();
            
            Tag *if_start = makeTag("if_stmt_%i", getCurrentContextBlockId(CTX_IF));
            setStartTagToCompoundLabel(program->if_node.reference_label, if_start);

            //Push an IF context 
            pushContext(CTX_IF);

            Value *condition = analyze(program->if_node.condition_expr, current_st, current_ft);

            analyze(program->if_node.body, current_st, current_ft);

            program->if_node.condition_size = calculateSizeOfType(condition->type);

            popContext();

            // Analyze the potential elseifs
            for (int i = 0; i < program->if_node.elseif_count; i++){
                int block_id = getCurrentContextBlockId(CTX_ELSEIF);
                analyze(program->if_node.elseif_nodes[i], current_st, current_ft);
                Tag *elseif_tag = makeTag("elseif_stmt_%i", block_id);
                //printf("MADE AN ELSEIF TAG : %s \n", elseif_tag->tag_name);
                addIntermediateTagToCompoundLabel(program->if_node.reference_label, elseif_tag);
            }

            /**
             * Analyze potential else
             */
            if (program->if_node.else_body != NULL){
                Tag *else_tag = makeTag("else_stmt_%i", getCurrentContextBlockId(CTX_ELSE));
                addIntermediateTagToCompoundLabel(program->if_node.reference_label, else_tag);
                pushContext(CTX_ELSE); 
                analyze(program->if_node.else_body, current_st, current_ft);
                popContext();

            }

            Tag *end_tag = makeTag("end_%s", if_start->tag_name);
            setEndTagToCompoundLabel(program->if_node.reference_label, end_tag);
            
            Value *return_value = makeVoidValue();
            return return_value;
        }

        case NODE_ELSEIF:
        {
            Value *condition = analyze(program->elseif_node.condition_expr, current_st, current_ft);
            //Push an ELSEIF context 
            pushContext(CTX_ELSEIF);

            analyze(program->elseif_node.body, current_st, current_ft);

            program->elseif_node.condition_size = calculateSizeOfType(condition->type);
            popContext();

            Value *return_value = makeVoidValue();
            return return_value;
        }

        case NODE_FOR:
        {
            program->for_node.label = makeCompoundLabel();
            Tag *for_start = makeTag("for_loop_%i", getCurrentContextBlockId(CTX_FOR));
            setStartTagToCompoundLabel(program->for_node.label, for_start);
            Tag *for_reassignment = makeTag("r_for_loop_%i", getCurrentContextBlockId(CTX_FOR));
            addIntermediateTagToCompoundLabel(program->for_node.label, for_reassignment);
            Tag *for_end = makeTag("end_%s", for_start->tag_name);
            setEndTagToCompoundLabel(program->for_node.label, for_end);

            //Increment current scope, we are inside the for loop
            pushContext(CTX_FOR); 
            pushCtxLabel(program->for_node.label);
            //Analyze the assignment
            analyze(program->for_node.assignment_expr, current_st, current_ft);
            //Analyze the condition
            analyze(program->for_node.condition_expr, current_st, current_ft);
            //Analyze the reassignment
            analyze(program->for_node.reassignment_expr, current_st, current_ft);
            //Analyze the body
            analyze(program->for_node.body, current_st, current_ft);

            //Decrement current scope, we are outside the for loop
            popContext(); 
            popCtxLabel();
            //Statements that return nothing
            Value *result = makeVoidValue();
            return result;
        }

        case NODE_FOREACH:
        {
            //Increment current scope, we are inside the foreach loop
            pushContext(CTX_FOREACH); 

            // Analyze the declaration, creates iterator
            Type *iterator_declaration_type = program->foreach_node.foreach_declaration->declaration_node.type;
            program->foreach_node.iterator_type = iterator_declaration_type;

            //analyze(program->foreach_node.foreach_declaration, current_st, current_ft);
            //Analyze the lower and upper limits, must be NUMBER
            Value *lower_limit = analyze(program->foreach_node.lower_limit_expr, current_st, current_ft);
            Value *upper_limit = analyze(program->foreach_node.upper_limit_expr, current_st, current_ft);

            if (lower_limit->type->kind != TYPE_NUMBER){
                Error *e = invoke(EXPECTED_TYPE_MUST_BE_NUMBER);
                appendMessageToError(e, fetchErrorMsgBasedOnErrorID(e->error_id), typeToString(lower_limit->type));
                appendMessageToError(e, "Foreach's lower limit must be a number.");
                addError(e);
            }
            if (upper_limit->type->kind != TYPE_NUMBER)
            {
                Error *e = invoke(EXPECTED_TYPE_MUST_BE_NUMBER);
                appendMessageToError(e, fetchErrorMsgBasedOnErrorID(e->error_id), typeToString(lower_limit->type));
                appendMessageToError(e, "Foreach's upper limit must be a number.");
                addError(e);
            }

            // Is the iterable expr actually iterable?
            Value *iterable_expr = analyze(program->foreach_node.iterable_expr, current_st, current_ft);
            // Store the iterable expression type in the AST
            program->foreach_node.iterable_expr_type = iterable_expr->type;
            //printf(" -------%s\n", program->foreach_node.iterable_expr_type);

            if (!isTypeAbleToSubscript(iterable_expr->type)){
                Error *e = invoke(TYPE_NOT_SUBSCRIPTABLE);
                appendMessageToError(e, fetchErrorMsgBasedOnErrorID(e->error_id), typeToString(iterable_expr->type));
                appendMessageToError(e, "Foreach's iterable expression must be suscriptable.");
                addError(e);
            }

            Type *element_type;
            if (isTypeOfKind(iterable_expr->type, TYPE_ARRAY)){
                element_type = iterable_expr->type->arrayType.element_type;
            }
            else if (isTypeOfKind(iterable_expr->type, TYPE_PTR)){
                element_type = iterable_expr->type->pointerType.pointed_type;
            }
            else if (isTypeOfKind(iterable_expr->type, TYPE_STR)){
                element_type = createNumberTypeFromInfo(1,1, NUMBER_CHAR);
            }
            else{
                fprintf(stderr, "I dont know man\n");
                exit(1);
            }

            // assignable instead?
            //Compare the iterator type and pointed type, must be equal
            if (!isTypeAssignableToDeclared(iterator_declaration_type, element_type)){
                fprintf(stderr, "Type mismatch in foreach statement. Iterator is of type: '%s', but iterable expression contains elements of type: '%s'\n",
                        typeToString(iterator_declaration_type), typeToString(element_type));
                exit(1);
            }

            //analyze(program->foreach_node.body, current_st, current_ft);

            //LOWER THE FOREACH NODE FOR BETTER BUILDING AND STORE IT IN THE AST
            // program->foreach_node.foreach_lowered = lowerForeach(program);

            program->foreach_node.foreach_lowered = lowerForeach(program);

            //print_ast(program->foreach_node.foreach_lowered,0);
            // printf("Analyzing foreach lowered\n");
            analyze(program->foreach_node.foreach_lowered, current_st, current_ft);
            // printf("Finished analyzing foreach lowered\n");
            //Decrement current scope
            popContext();
            //Statements that return nothing
            Value *result = makeVoidValue();
            return result;

        }

        case NODE_CAST:
        {
            analyze(program->cast_node.expr, current_st, current_ft);
            Value *casting_type = makeSpecificValue(program->cast_node.castType);
            return casting_type;
        }

        case NODE_SIZEOF:
        {

            Type *result_type = NULL;
            if (program->sizeof_node.sizeof_type == SIZEOF_EXPR){
                result_type = analyze(program->sizeof_node.expr, current_st, current_ft)->type;
            } 
            else{
                result_type = program->sizeof_node.type;
            }

            program->sizeof_node.result_type = result_type;
            int exists = doesTypeExist(program->sizeof_node.result_type);
            
            if (!exists){
                Error *e = invoke(TYPE_UNDEFINED);
                appendMessageToError(e, fetchErrorMsgBasedOnErrorID(e->error_id), typeToString(program->sizeof_node.result_type));
                addError(e);
            }

            if (isTypeOfKind(program->sizeof_node.result_type, TYPE_ARRAY) && program->sizeof_node.result_type->arrayType.size_provided_kind == ARR_SIZE_INTERPRETED 
                    && program->sizeof_node.result_type->arrayType.n_of_elements == 0){
                Error *e = invoke(TYPE_INCOMPLETE);
                appendMessageToError(e, fetchErrorMsgBasedOnErrorID(e->error_id), typeToString(program->sizeof_node.result_type));
                appendMessageToError(e, "Can't calculate the size of an incomplete type.");
                addError(e);
            }

            // CHECK FW TYPES
            if (isTypeOfKind(result_type, TYPE_UDT)){
                if (result_type->udt.udt_kind == UDT_CLASS){
                    Class *c = lookUpClass(gb_classTable, result_type->udt.name);
                    if (c->is_forward){
                        Error *e = invoke(TYPE_INCOMPLETE);
                        appendMessageToError(e, fetchErrorMsgBasedOnErrorID(e->error_id), typeToString(program->sizeof_node.result_type));
                        appendMessageToError(e, "Can't calculate the size of an incomplete type.");
                        addError(e);
                    }
                }
                else if (result_type->udt.udt_kind == UDT_OBJECT){
                    Object *o = lookUpObject(gb_objectTable, result_type->udt.name);
                    if (o->is_forward){
                        Error *e = invoke(TYPE_INCOMPLETE);
                        appendMessageToError(e, fetchErrorMsgBasedOnErrorID(e->error_id), typeToString(program->sizeof_node.result_type));
                        appendMessageToError(e, "Can't calculate the size of an incomplete type.");
                        addError(e);
                    }
                }
            }

            Type *sizeof_type = createNumberTypeFromInfo(8,1, NUMBER_INTEGER);   // uint64
            Value *result = makeSpecificValue(sizeof_type);
            return result;
        }

        case NODE_FUNC_CALL:
        {
            

            Type **param_expr_types = malloc(sizeof(Type *) * program->funccall_node.params_count);

            for (int i = 0; i < program->funccall_node.params_count; i++){
                param_expr_types[i] = analyze(program->funccall_node.params_expr[i], current_st, current_ft)->type;
            }

            /*
            Find the function based on name and types provided in the func call
            */

            //print_functionTable(current_ft);
            Function *f = lookUpFunction(current_ft, getCurrentScope(), program->funccall_node.identifier, param_expr_types, program->funccall_node.params_count);


            if (f == NULL){
                Error *e = invoke(FUNCTION_NOT_FOUND);
                appendMessageToError(e, fetchErrorMsgBasedOnErrorID(e->error_id), program->funccall_node.identifier);
                appendMessageToError(e, "Types given were: \n(");
                for (int i = 0; i < program->funccall_node.params_count; i++){
                    appendMessageToError(e, "%s", typeToString(param_expr_types[i]));
                    if (i < program->funccall_node.params_count - 1){
                        appendMessageToError(e, ",");
                    }
                }
                appendMessageToError(e, ")\n");
                addError(e);
            }
            program->funccall_node.params_type = param_expr_types;
            program->funccall_node.function = f;

            Value *returned_value = makeSpecificValue(f->fmi->rt_type);
            return returned_value;
        }

        case NODE_METHOD_DISPATCH:
        {
            Value *base = analyze(program->method_dispatch.base, current_st, current_ft);

            if (base->type->kind == TYPE_PTR){
                Error *e = invoke(TYPE_UNEXPECTED);
                appendMessageToError(e, fetchErrorMsgBasedOnErrorID(e->error_id), typeToString(base->type));
                appendMessageToError(e, "Base must be of NON pointer type when performing a method dispatch through '.', use '->' instead.");
                addError(e);
            }

            Class *c = lookUpClass(gb_classTable, base->type->udt.name);
            if (c == NULL){
                Error *e = invoke(CLASS_UNDEFINED);
                appendMessageToError(e, fetchErrorMsgBasedOnErrorID(e->error_id), base->type->udt.name);
                appendMessageToError(e, "Could not resolve method dispatch of method: '%s'.", program->method_dispatch.func_call->funccall_node.identifier);
                addError(e);
            }
            //printf("Class function table: \n");
            //print_functionTable(c->local_functions);
            program->method_dispatch.base_type = base->type;

            program->method_dispatch.md_lowered = lowerMethodDispatch(program);
            Value *func_call = analyze(program->method_dispatch.md_lowered, current_st, c->local_functions);

            return func_call;
        }

        case NODE_PTR_METHOD_DISPATCH:
        {
            Value *base = analyze(program->ptr_method_dispatch.base, current_st, current_ft);
            if (base->type->kind != TYPE_PTR){
                Error *e = invoke(TYPE_UNEXPECTED);
                appendMessageToError(e, fetchErrorMsgBasedOnErrorID(e->error_id), typeToString(base->type));
                appendMessageToError(e, "Base must be of pointer type when performing a method dispatch through '->', use '.' instead.");
                addError(e);
            }
            Type *pointed_type = base->type->pointerType.pointed_type;
            Class *c = lookUpClass(gb_classTable, pointed_type->udt.name);

            if (c == NULL){
                Error *e = invoke(CLASS_UNDEFINED);
                appendMessageToError(e, fetchErrorMsgBasedOnErrorID(e->error_id), pointed_type->udt.name);
                appendMessageToError(e, "Could not resolve pointer method dispatch of method: '%s'.", program->ptr_method_dispatch.func_call->funccall_node.identifier);
                addError(e); 
            }
            
            program->ptr_method_dispatch.base_type = base->type;

            //Analyze every expr 
            Type **param_types = malloc(100 * sizeof(Type *));

            for (int i = 0; i < program->ptr_method_dispatch.func_call->funccall_node.params_count; i++){
                param_types[i] = analyze(program->ptr_method_dispatch.func_call->funccall_node.params_expr[i], current_st, c->local_functions)->type;
            }

            program->ptr_method_dispatch.func_call->funccall_node.params_type = param_types;


            program->ptr_method_dispatch.ptrmd_lowered = lowerPtrMethodDispatch(program);

            // printf("About to analyze lower ptr method dispatch\n");
            // printf("ptrmd_lowered = %p\n", program->ptr_method_dispatch.ptrmd_lowered);
            // printf("ptrmd_lowered->node_type = %s\n", astTypeToStr(program->ptr_method_dispatch.ptrmd_lowered));
            // printf("current_st = %p\n", current_st);

            Value *func_call = analyze(program->ptr_method_dispatch.ptrmd_lowered, current_st, c->local_functions);

            return func_call;
        }

        case NODE_UNION:
        {
            pushContext(CTX_UNION);

            Symbol **symbols = malloc(sizeof(Symbol *) * program->union_node.declaration_count);
            Symbol *max_symbol = NULL;

            Object *o = getCurrentObject();
            for (int i = 0; i < program->union_node.declaration_count; i++){
                Type *decl_type = program->union_node.declarations[i]->declaration_node.type;
                if (isTypeOfKind(decl_type, TYPE_UDT) && decl_type->udt.udt_kind == UDT_OBJECT && strcmp(decl_type->udt.name, o->identifier) == 0){
                    Error *e = invoke(FIELD_WITH_INCOMPLETE_TYPE);
                    appendMessageToError(e, fetchErrorMsgBasedOnErrorID(e->error_id), 
                                        program->object_node.declarations[i]->declaration_node.identifier, typeToString(decl_type));
                    appendMessageToError(e, "Withion union context. Consider using a pointer instead\n");
                    addError(e);
                }
                
                // Analyze the declarations for proper types 
                analyze(program->union_node.declarations[i], current_st, current_ft);

                /** Manually create the symbols */
                Symbol *s = makeLocalVarSymbol(program->union_node.declarations[i]->declaration_node.identifier, 
                                                program->union_node.declarations[i]->declaration_node.type);
                
                /** Save the biggest symbol to add last */
                if (max_symbol == NULL || s->smi->size > max_symbol->smi->size){
                    max_symbol = s;
                }

                /** Log the symbols for later, tweak the symbols as they were created within the UNION contex */
                symbols[i] = s;
                s->smi->def_ctx = context.ctx_block_stack[context.current_scope - 1];
                s->smi->scope--;
            }
            int offset = align_to(current_st->symbols[current_st->count - 1]->smi->offset + current_st->symbols[current_st->count - 1]->smi->size,
                                  max_symbol->smi->type);
            
            for (int k = 0; k < program->union_node.declaration_count; k++){
                symbols[k]->smi->offset = offset;
            }

            /** Add the biggest one last, so the next symbol in the object/class has an offset at "max_size" distance of the union symbols **/
            for (int i = 0; i < program->union_node.declaration_count; i++){
                if (current_st->count + 1 >= current_st->capacity){
                        __resizeSymbolTable(current_st);
                }

                if (max_symbol != symbols[i]){
                    for (int j = 0; j < current_st->count; j++){
                        if (doSymbolsClash(symbols[i], current_st->symbols[j])){
                            Error *e = invoke(REDEFINITION_OF_VARIABLE);
                            appendMessageToError(e, fetchErrorMsgBasedOnErrorID(e->error_id), symbols[i]->smi->identifier);
                            appendMessageToError(e, "Previous definition of identifier: '%s', in file: '%s', in L = %i.", 
                                current_st->symbols[i]->smi->identifier, current_st->symbols[j]->smi->def_src_file, current_st->symbols[j]->smi->def_line_number);
                            addError(e);
                        }
                    }
                    current_st->symbols[current_st->count] = symbols[i];
                    symbols[i]->smi->index = current_st->count++;
                }
            }

            /** Add the biggest one */
            if (current_st->count + 1 >= current_st->capacity){
                        __resizeSymbolTable(current_st);
            }
            for (int i = 0; i < current_st->count; i++){
            
                if (doSymbolsClash(max_symbol, current_st->symbols[i])){
                    Error *e = invoke(REDEFINITION_OF_VARIABLE);
                    appendMessageToError(e, fetchErrorMsgBasedOnErrorID(e->error_id), max_symbol->smi->identifier);
                    appendMessageToError(e, "Previous definition in file: '%s', in L = %i.", current_st->symbols[i]->smi->def_src_file, current_st->symbols[i]->smi->def_line_number);
                    addError(e);
                }
            }
            max_symbol->smi->index = current_st->count;
            current_st->symbols[current_st->count++] = max_symbol;

            /**
             * Compute the padding
             */
            current_st->padding = 0;
            for (int i = 1; i < current_st->count; i++){
                if (current_st->symbols[i]->smi->offset == current_st->symbols[i - 1]->smi->offset){
                    continue;
                }
                current_st->padding += current_st->symbols[i]->smi->offset - (current_st->symbols[i - 1]->smi->offset + current_st->symbols[i - 1]->smi->size);
            }

            popContext();


            Value *v = makeVoidValue();
            return v;
        }

        case NODE_OBJECT:
        {

            /**
             * Find possible object parent
             */

            Object *parent = NULL;
            if (program->object_node.parent != NULL){
                parent = lookUpObject(gb_objectTable, program->object_node.parent);
            }

            /**
             * Create the object and absorb the parent's symbols into its symbol table -> Parent fields come first
             */
            Object *o = makeObject(program->object_node.identifier, parent, program->object_node.is_forward, program->object_node.object_type);
            if (parent != NULL){
                // printf("Absoring symbols to '%s' from parent '%s', with total count = %i\n", o->identifier, parent->identifier, parent->local_symbols->count);
                absorbSymbolsIntoTable(o->local_symbols, parent->local_symbols);
            }
            addObjectToTable(gb_objectTable, o);


            if (program->object_node.is_forward == 1){
                Value *result = makeVoidValue();
                return result;
            }

            pushContext(CTX_OBJECT);

            context.current_object = o;
            /**
             * Analyze every field declaration
             */
            for (int i = 0; i < program->object_node.declaration_count; i++){
                // An object cannot have a field that is its own type by value
                if (program->object_node.declarations[i]->node_type == NODE_DECLARATION){
                    Type *decl_type = program->object_node.declarations[i]->declaration_node.type;
                    if (isTypeOfKind(decl_type, TYPE_UDT) && decl_type->udt.udt_kind == UDT_OBJECT && strcmp(decl_type->udt.name, o->identifier) == 0){
                        Error *e = invoke(FIELD_WITH_INCOMPLETE_TYPE);
                        appendMessageToError(e, fetchErrorMsgBasedOnErrorID(e->error_id), 
                                            program->object_node.declarations[i]->declaration_node.identifier, typeToString(decl_type));
                        appendMessageToError(e, "Consider using a pointer instead\n");
                        addError(e);
                    }
                }

                // Analyze declarations and union
                analyze(program->object_node.declarations[i], o->local_symbols, current_ft);
            }

            context.current_object = NULL;
            popContext();
            Value *result = makeVoidValue();

            //print_symbolTable(o->local_symbols);

            return result;
        }

        case NODE_CLASS:
        {
            //printf("Scope is now: %i\n", context.current_scope);
            /**
             * Find possible class parent
             */
            Class *parent = NULL;
            if (program->class_node.parent != NULL){
                parent = lookUpClass(gb_classTable, program->class_node.parent);
            }

            /**
             * Create the class and absorb the parent's symbols into its symbol table -> Parent fields come first
             */

            Class *c = makeClass(program->class_node.identifier, parent, program->class_node.is_forward, program->class_node.class_type);

            if (parent != NULL){
                absorbSymbolsIntoTable(c->local_symbols, parent->local_symbols);
            }

            addClassToTable(gb_classTable, c);

            if (c->is_forward == 1){
                Value *result = makeVoidValue();
                //printTypeTable();
                return result;
            }

            context.current_class = c;
            pushContext(CTX_CLASS);

            /**
             * Analyze every field declaration and function def in the class
             */
            for (int i = 0; i < program->class_node.declaration_count; i++){
                if (program->class_node.declarations[i]->node_type == NODE_DECLARATION){
                    // A class cannot have a field that is its own type by value
                    Type *decl_type = program->class_node.declarations[i]->declaration_node.type;
                    if (isTypeOfKind(decl_type, TYPE_UDT) && decl_type->udt.udt_kind == UDT_CLASS && strcmp(decl_type->udt.name, c->identifier) == 0){
                        Error *e = invoke(FIELD_WITH_INCOMPLETE_TYPE);
                        appendMessageToError(e, fetchErrorMsgBasedOnErrorID(e->error_id), 
                                            program->class_node.declarations[i]->declaration_node.identifier, typeToString(decl_type));
                        appendMessageToError(e, "Consider using a pointer instead\n");
                        addError(e);
                    }
                }

                // Analyze every field declaration (unions too) and func def
                analyze(program->class_node.declarations[i], c->local_symbols, c->local_functions);
            }

            for (int i = 0; i < program->class_node.funcdefs_count; i++){
                analyze(program->class_node.func_defs[i], c->local_symbols, c->local_functions);
            }

            popContext();
            context.current_class = NULL;
            Value *result = makeVoidValue();
            //printTypeTable();
            return result;
        }

        case NODE_ENUM:
        { 
            /** 19/1 --> Dont push context for enums, since the declarations there are global */
            //pushContext(CTX_ENUM);
            Enum *e = makeEnum(program->enum_node.identifier, program->enum_node.is_forward, program->enum_node.enum_type, program->enum_node.base_type);

            addEnumToTable(gb_enumTable, e);

            if (e->is_forward == 1){
                Value *result = makeVoidValue();
                return result;
            }
            
            // Base type needs to be a number 
            if (!(isTypeOfKind(program->enum_node.base_type, TYPE_NUMBER) && program->enum_node.base_type->numberType.kind == NUMBER_INTEGER)){
                Error *e = invoke(TYPE_UNEXPECTED);
                appendMessageToError(e, fetchErrorMsgBasedOnErrorID(e->error_id), typeToString(program->enum_node.base_type));
                appendMessageToError(e, "Enum base type must be a number type");
                addError(e);
            }

            for (int i = 0; i < program->enum_node.declaration_count; i++){
                ASTNode *current_decl = program->enum_node.declarations[i];
                switch(program->enum_node.declarations[i]->node_type)
                {
                    case NODE_IDENTIFIER:{
                        // Add the symbol to gb table
                        Symbol *s = makeGlobalSymbol(current_decl->identifier_node.name, program->enum_node.enum_type);
                        addSymbolToTable(s, gb_symbolTable);
                        break;
                    }
                    case NODE_REASSIGNMENT:{
                        // Must have ID = NUMBER;
                        if (current_decl->reassignment_node.lvalue->node_type != NODE_IDENTIFIER){
                            Error *e = invoke(ILL_FORMATED_ENUM);
                            appendMessageToError(e, fetchErrorMsgBasedOnErrorID(e->error_id));
                            appendMessageToError(e, "Got '%s' instead of identifier.", astTypeToStr(current_decl));
                            addError(e);
                        }
                        // Analyze the expr
                        // Error if the expr is not assignable to the base type of the enum
                        Value *expr_result = analyze(current_decl->reassignment_node.expression, current_st, current_ft);
                        if (!isTypeAssignableToDeclared(program->enum_node.base_type, expr_result->type)){
                            Error *e = invoke(RTYPE_CANT_BE_ASSIGNED_TO_LTYPE);
                            appendMessageToError(e, fetchErrorMsgBasedOnErrorID(e->error_id), typeToString(program->enum_node.base_type), typeToString(expr_result->type));
                            addError(e);
                        }
                        
                        // printf("During analysis, the number is: %i\n", current_decl->reassignment_node.expression->number_node.number_value);
                        char *sy_name = current_decl->reassignment_node.lvalue->identifier_node.name;
                        // At this point we have ID = NUMBER;
                        Symbol *s = makeGlobalSymbol(sy_name, program->enum_node.enum_type);
                        addSymbolToTable(s, gb_symbolTable);
                        break;
                    }
                    default:{
                        Error *e = invoke(ILL_FORMATED_ENUM);
                        appendMessageToError(e, fetchErrorMsgBasedOnErrorID(e->error_id));
                        appendMessageToError(e, "Got '%s' instead of identifier.", astTypeToStr(current_decl));
                        addError(e);
                        break;
                    }
                }
            }
            
            Value *result = makeVoidValue();
            return result;
        }

        case NODE_BINARY_OP:
        {
            /**
             * Analyze and validate the Binary Operation
             */
            Value *l = analyze(program->binary_op_node.left, current_st, current_ft);
            Value *r = analyze(program->binary_op_node.right, current_st, current_ft);
           
            BinOPCode bin_op = strToBinOPCode(program->binary_op_node.op);
            validateBinaryOP(l->type, r->type, bin_op);

            /**
             * Promote the resultant type to the biggest one
             */
            Type *result_type = performBinaryOnTypes(l->type, r->type, bin_op);
            
            Value *result = makeSpecificValue(result_type);
            return result;
        }

        case NODE_RETURN:
        {

            Value *return_value = analyze(program->return_node.return_expr, current_st, current_ft);
            return return_value;

        }

        case NODE_ARRAY_INIT:
        {
            Type *expected_element_type = program->array_init_node.type->arrayType.element_type;
            for (int i = 0; i < program->array_init_node.element_count; i++){
                // printf("Array init of %i elements\n", program->array_init_node.element_count);
                Value *element = analyze(program->array_init_node.elements[i], current_st, current_ft);   
                
                if (!isTypeAssignableToDeclared(expected_element_type, element->type)){
                    Error *e = invoke(ARRAY_TYPE_MIXING);
                    appendMessageToError(e, fetchErrorMsgBasedOnErrorID(e->error_id));
                    appendMessageToError(e, "Expected type for all elements is: '%s'. But at index: %i, got type: '%s'.", typeToString(expected_element_type),
                                         i, typeToString(element->type));
                    addError(e);
                }
            }

            if (program->array_init_node.type->arrayType.size_provided_kind == ARR_SIZE_INTERPRETED){
            //     printf("Type: %s, at %p. We are populating its n of elements to : %i\n", typeToString(program->array_init_node.type),
            // program->array_init_node.type, program->array_init_node.element_count);
                //program->array_init_node.type->arrayType.n_of_elements = program->array_init_node.element_count;
                program->array_init_node.type = createArrayTypeFromInfo(program->array_init_node.element_count, expected_element_type);
                // printf("Populated the type: %s\n", typeToString(program->array_init_node.type));
                // printTypeTable();
            }
            else{
                if (program->array_init_node.type->arrayType.n_of_elements != program->array_init_node.element_count){
                    Error *e = invoke(ARRAY_SIZE_MISMATCH);
                    appendMessageToError(e, fetchErrorMsgBasedOnErrorID(e->error_id));
                    appendMessageToError(e, "During array init, declared number of elements is: %i, but got: %i.", 
                                        program->array_init_node.type->arrayType.n_of_elements, program->array_init_node.element_count);
                    addError(e);
                }
            }

            Value *result = makeSpecificValue(program->array_init_node.type);
            return result;
        }
        
        case NODE_SUBSCRIPT:
        {

            Value *base = analyze(program->subscript_node.base, current_st, current_ft);

            if (!isTypeAbleToSubscript(base->type))
            {
                Error *e = invoke(TYPE_NOT_SUBSCRIPTABLE);
                appendMessageToError(e, fetchErrorMsgBasedOnErrorID(e->error_id), typeToString(base->type));
                addError(e);
            }


            Value *index = analyze(program->subscript_node.index, current_st, current_ft);
            if (!isTypeOfKind(index->type, TYPE_NUMBER)){
                Error *e = invoke(EXPECTED_TYPE_MUST_BE_NUMBER);
                appendMessageToError(e, fetchErrorMsgBasedOnErrorID(e->error_id), typeToString(index->type));
                appendMessageToError(e, "Subscript index must be of type number.");
                addError(e);
            }

            Type *element_type;

            switch(base->type->kind)
            {
                case TYPE_ARRAY:{
                    element_type = base->type->arrayType.element_type;
                    break;
                }
                case TYPE_PTR:{
                    element_type = base->type->pointerType.pointed_type;
                    break;
                }
                case TYPE_STR:{
                    element_type = createNumberTypeFromInfo(1, 1, NUMBER_CHAR);  //Char
                    break;
                }
                default:{
                    fprintf(stderr, "Type is not able to subscript: '%s'\n", typeToString(base->type));
                    exit(1);
                }
            }
            /**
             * Store the base type, element size and index size in the ASTNode
             */
            int element_size = calculateSizeOfType(element_type);
            program->subscript_node.base_type = base->type;
            program->subscript_node.element_size = element_size; 
            program->subscript_node.index_size = index->size;
            

            // We want to return the array element type
            Value *subscript_result = makeSpecificValue(element_type);

            return subscript_result;
        }
        
        case NODE_ASSIGNMENT:
        {

            /**
             * Does the type exist to begin with
             */
            int exists = doesTypeExist(program->assignment_node.type);
            if (!exists){
                Error *e = invoke(TYPE_UNDEFINED);
                appendMessageToError(e, fetchErrorMsgBasedOnErrorID(e->error_id), typeToString(program->sizeof_node.type));
                addError(e);
            }

            program->assignment_node.type->representation = updateTypeRepresentation(program->assignment_node.type);
            /**
             * Analyze the expression and see if the ExprType is assignable to DeclaredType
             */

            Value *v = analyze(program->assignment_node.expression, current_st, current_ft);



            /**
             * Special case for array init ---> type[size?] = array_init
             */
            if (isTypeOfKind(program->assignment_node.type, TYPE_ARRAY) && program->assignment_node.expression->node_type == NODE_ARRAY_INIT){

                if (program->assignment_node.type->arrayType.size_provided_kind == ARR_SIZE_INTERPRETED){
                    //printf("Node at line = %i,used to have type: %s, ", program->assignment_node.line_number, typeToString(program->assignment_node.type));
                    // Created another reference to the type so this one has to link to the new non [0] type
                    program->assignment_node.type = program->assignment_node.expression->array_init_node.type;
                    //printf("but now has: %s\n",typeToString(program->assignment_node.type) );
                }

                if (program->assignment_node.type->arrayType.size_provided_kind == ARR_SIZE_GIVEN && 
                    program->assignment_node.type->arrayType.n_of_elements != v->type->arrayType.n_of_elements){
                    Error *e = invoke(ARRAY_SIZE_MISMATCH);
                    appendMessageToError(e, fetchErrorMsgBasedOnErrorID(e->error_id));
                    appendMessageToError(e, "Array init expression yielded: %i, elements but declared expected: %i, elements.", 
                                        v->type->arrayType.n_of_elements, program->assignment_node.type->arrayType.n_of_elements);
                    addError(e);
                }
            }
            else {
                int match = isTypeAssignableToDeclared(program->assignment_node.type, v->type);
                if (!match){
                    Error *e = invoke(RTYPE_CANT_BE_ASSIGNED_TO_LTYPE);
                    appendMessageToError(e, fetchErrorMsgBasedOnErrorID(e->error_id), typeToString(program->assignment_node.type), typeToString(v->type));
                    addError(e);
                }
            }

            /**
             * Create the symbol and add it to the table
             */
            Symbol *s;
            if (getCurrentContextType() == CTX_NONE){
                s = makeGlobalSymbol(program->assignment_node.identifier, program->assignment_node.type);
            }
            else{
                s = makeLocalVarSymbol(program->assignment_node.identifier, program->assignment_node.type);
            }

            addSymbolToTable(s, current_st);
            program->assignment_node.s = s;
            Value *none = makeVoidValue();
            return none;
        }

        case NODE_EXTERN_IDENTIFIER:
        {
            if (getCurrentContextType() != CTX_NONE){
                Error *e = invoke(EXTERN_ID_DEF_ONLY_ALLOWED_GLOBALLY);
                appendMessageToError(e, fetchErrorMsgBasedOnErrorID(e->error_id), program->extern_identifier_node.name);
                addError(e);
            }

            /**
             * Does the type exist to begin with
             */
            int exists = doesTypeExist(program->extern_identifier_node.type);
            if (!exists){
                Error *e = invoke(TYPE_UNDEFINED);
                appendMessageToError(e, fetchErrorMsgBasedOnErrorID(e->error_id), typeToString(program->extern_identifier_node.type));
                addError(e);
            }

            Symbol *s = makeExternSymbol(program->extern_identifier_node.name, program->extern_identifier_node.type);
            addSymbolToTable(s, current_st);
            Value *none = makeVoidValue();
            return none;
        }

        case NODE_REASSIGNMENT:
        {
            /**
             * Analyze both lvalue and rvalue.
             * Store the lvalue Type in the ASTNode
             */
            if (!isLvalue(program->reassignment_node.lvalue)){
                Error *e = invoke(INVALID_LVALUE);
                appendMessageToError(e, fetchErrorMsgBasedOnErrorID(e->error_id), astTypeToStr(program->reassignment_node.lvalue));
                addError(e);
            }


            Value *lvalue = analyze(program->reassignment_node.lvalue, current_st, current_ft);
            Value *rvalue = analyze(program->reassignment_node.expression,current_st, current_ft);

            program->reassignment_node.type = lvalue->type;
            program->reassignment_node.size = calculateSizeOfType(program->reassignment_node.type);

            //printf("Comparing between %s and %s \n", s->type, v.type);
            validateReassignOperationWithTypes(lvalue->type, rvalue->type, program->reassignment_node.op);

            //Value to ret for statements with no return value
            Value *none = makeVoidValue();
            //printf("returning from reassignment with type: %s \n",ret.type );
            return none;
        }

        case NODE_FIELD_ACCESS:
        {

            Value *base = analyze(program->field_access_node.base, current_st, current_ft);
            program->field_access_node.type = base->type;

            if (isTypeOfKind(base->type, TYPE_PTR)){
                Error *e = invoke(TYPE_UNEXPECTED);
                appendMessageToError(e, fetchErrorMsgBasedOnErrorID(e->error_id), typeToString(base->type));
                appendMessageToError(e, "Base must be of NON pointer type when performing a field access through '.', use '->' instead.");
                addError(e);
            }

            if (!isTypeOfKind(base->type, TYPE_UDT)){
                Error *e = invoke(TYPE_UNEXPECTED);
                appendMessageToError(e, fetchErrorMsgBasedOnErrorID(e->error_id), typeToString(base->type));
                appendMessageToError(e, "Base pointed type must be of UDT type pointer type when performing a field access through '.'");
                addError(e);
            }

            Symbol *field;

            switch (base->type->udt.udt_kind)
            {
                case UDT_CLASS:
                {
                    Class *c = lookUpClass(gb_classTable, base->type->udt.name);
                    field = lookUpField(c->local_symbols, program->field_access_node.field_name);
                    if (field == NULL){
                        Error *e = invoke(FIELD_NOT_DEFINED_IN_CLASS);
                        appendMessageToError(e, fetchErrorMsgBasedOnErrorID(e->error_id), program->field_access_node.field_name, c->identifier);
                        addError(e);
                    }
                    break;
                }
                case UDT_OBJECT:
                {
                    Object *o = lookUpObject(gb_objectTable, base->type->udt.name);
                    field = lookUpField(o->local_symbols, program->field_access_node.field_name);
                    if (field == NULL){
                        Error *e = invoke(FIELD_NOT_DEFINED_IN_OBJECT);
                        appendMessageToError(e, fetchErrorMsgBasedOnErrorID(e->error_id), program->field_access_node.field_name, o->identifier);
                        addError(e);
                    }
                    break;
                }

                case UDT_ENUM:{
                    Error *e = invoke(ENUM_DOESNT_HAVE_FIELDS);
                    appendMessageToError(e, fetchErrorMsgBasedOnErrorID(e->error_id), base->type->udt.name);
                    addError(e);
                }
                default:{
                    fprintf(stderr, "Trying to access a field off of a type that is not UDT.\n");
                    exit(1);
                }
            }

            Value *field_access = makeSpecificValue(field->smi->type);
            return field_access;
        }
    
        case NODE_PTR_FIELD_ACCESS:
        {
            Value *base = analyze(program->ptr_field_access_node.base, current_st, current_ft);

            if (!isTypeOfKind(base->type, TYPE_PTR)){
                Error *e = invoke(TYPE_UNEXPECTED);
                appendMessageToError(e, fetchErrorMsgBasedOnErrorID(e->error_id), typeToString(base->type));
                appendMessageToError(e, "Base must be of pointer type when performing a field access through '->', use '.' instead.");
                addError(e);
            }

            Type *pointed_type = base->type->pointerType.pointed_type;
            program->ptr_field_access_node.type = pointed_type;


            if (!isTypeOfKind(pointed_type, TYPE_UDT)){
                Error *e = invoke(TYPE_UNEXPECTED);
                appendMessageToError(e, fetchErrorMsgBasedOnErrorID(e->error_id), typeToString(base->type));
                appendMessageToError(e, "Base pointed type must be of UDT type when performing a field access through '->'.");
                addError(e);
            }

            Symbol *field;

            switch (pointed_type->udt.udt_kind)
            {
                case UDT_CLASS:{
                    Class *c = lookUpClass(gb_classTable, pointed_type->udt.name);
                    field = lookUpField(c->local_symbols, program->ptr_field_access_node.field_name);
                    if (field == NULL){
                        Error *e = invoke(FIELD_NOT_DEFINED_IN_CLASS);
                        appendMessageToError(e, fetchErrorMsgBasedOnErrorID(e->error_id), program->ptr_field_access_node.field_name, c->identifier);
                        addError(e);
                    }
                    break;
                }
                case UDT_OBJECT:{
                    Object *o = lookUpObject(gb_objectTable, pointed_type->udt.name);
                    field = lookUpField(o->local_symbols, program->ptr_field_access_node.field_name);
                    if (field == NULL){
                        Error *e = invoke(FIELD_NOT_DEFINED_IN_OBJECT);
                        appendMessageToError(e, fetchErrorMsgBasedOnErrorID(e->error_id), program->ptr_field_access_node.field_name, o->identifier);
                        addError(e);
                    }
                    break;
                }

                case UDT_ENUM:{
                    Error *e = invoke(ENUM_DOESNT_HAVE_FIELDS);
                    appendMessageToError(e, fetchErrorMsgBasedOnErrorID(e->error_id), base->type->udt.name);
                    addError(e);
                }
                default:{
                    fprintf(stderr, "Trying to pointer access a field off of a type that is not UDT.\n");
                    exit(1);
                }
            }

            Value *ptr_field_access = makeSpecificValue(field->smi->type);
            return ptr_field_access;
        }

        case NODE_IDENTIFIER:
        {
            Symbol *s = lookUpSymbol(current_st, context.current_scope, program->identifier_node.name);
            if (s == NULL){
                Error *e = invoke(VARIABLE_DEFINITION_NOT_FOUND);
                appendMessageToError(e, fetchErrorMsgBasedOnErrorID(e->error_id), program->identifier_node.name);
                addError(e);
            }
            if (getCurrentContextType() == CTX_NONE){
                Error *e = invoke(GLOBAL_ELEMENT_NOT_ALLOWED);
                appendMessageToError(e, fetchErrorMsgBasedOnErrorID(e->error_id));
                appendMessageToError(e, "Variable '%s', is not allowed within an expression in global context, which requires everything to be known at compile time.", 
                                    s->smi->identifier);
                addError(e);
            }
            program->identifier_node.s = s;
            Value *id = makeSpecificValue(s->smi->type);
            return id;
        }

        case NODE_NUMBER:{
            Value *number = makeSpecificValue(program->number_node.number_type);
            return number;
        }

        case NODE_CHAR:{
            Value *ch = makeSpecificValue(program->char_node.char_type);
            return ch;
        }


        case NODE_BOOL:{
            Value *b = makeSpecificValue(program->bool_node.bool_type);
            return b;
        }

        case NODE_UNARY_OP:
        {
            //Some unary operations change the type of the result
            Value *right = analyze(program->unary_op_node.right, current_st, current_ft);

            validateUnaryOP(right->type, program->unary_op_node.op);

            Type *type_after = performUnaryOnType(right->type, program->unary_op_node.op);
            //program->unary_op_node.size_of_operand = calculateSizeOfType(right->type);
            if (isTypeOfKind(type_after, TYPE_VOID)){
                program->unary_op_node.size_of_operand = 8;
            }
            else{
                program->unary_op_node.size_of_operand = calculateSizeOfType(type_after);
            }
            Value *result = makeSpecificValue(type_after);

            return result;
        }

        case NODE_POSTFIX_OP:
        {

            Value *operand = analyze(program->postfix_op_node.left, current_st, current_ft);
            program->postfix_op_node.type = operand->type;
            program->postfix_op_node.size_of_operand = calculateSizeOfType(operand->type);

            Value *result = makeVoidValue();
            if (!program->is_stmt){
                result = operand;
            }
            return result;
        }   

        case NODE_STR:
        {
            //Add the string to the table and create the label
            addStrToTable(gb_stringTable, program->str_node.str_value);

            // Log the label in the AST node
            program->str_node.label = getStringTableLabelAt(gb_stringTable, gb_stringTable->count - 1);
            
            Value *str = makeSpecificValue(program->str_node.str_type);
            return str;
        }

        case NODE_DECLARATION:
        {
            /**
             * Does the type exist to begin with
             */

            int exists = doesTypeExist(program->declaration_node.type);

            if (!exists){
                Error *e = invoke(TYPE_UNDEFINED);
                appendMessageToError(e, fetchErrorMsgBasedOnErrorID(e->error_id), typeToString(program->sizeof_node.type));
                addError(e);
            }

            program->declaration_node.type->representation = updateTypeRepresentation(program->declaration_node.type);

            if (isTypeOfKind(program->declaration_node.type, TYPE_ARRAY) && program->declaration_node.type->arrayType.size_provided_kind == ARR_SIZE_INTERPRETED){
                Error *e = invoke(TYPE_INCOMPLETE);
                appendMessageToError(e, fetchErrorMsgBasedOnErrorID(e->error_id), typeToString(program->declaration_node.type));
                appendMessageToError(e, "Can't provide an interpreted size on an array declaration.");
                addError(e);
            }

            /** If not in UNION, crate the symbol and add it to the table
             *  If we are in an UNION, we create the symbol manually
             */

            if (getCurrentContextType() != CTX_UNION){
                Symbol *s = NULL;
                if (getCurrentContextType() == CTX_NONE){
                    s = makeGlobalSymbol(program->declaration_node.identifier, program->declaration_node.type);
                }
                else{
                    s = makeLocalVarSymbol(program->declaration_node.identifier, program->declaration_node.type);
                }

                addSymbolToTable(s, current_st);
            }
            
            Value *none = makeVoidValue();
            return none;
        }

        default:{
            Value *none = makeVoidValue();
            return none;
        }

    }
}


void print_symbolTable(SymbolTable *st)
{
    printf("============================================================================================================\n");
    printf("%-16s %-50s %-8s %-8s %-8s\n", "Name", "Type", "Offset", "Index", "Scope");
    printf("------------------------------------------------------------------------------------------------------------\n");

    for (int i = 0; i < st->count; i++)
    {
        printf("%-16s %-50s %-8i %-8i %-8i\n",
               st->symbols[i]->smi->identifier,
               typeToString(st->symbols[i]->smi->type),
               st->symbols[i]->smi->offset,
               st->symbols[i]->smi->index,
               st->symbols[i]->smi->scope);
    }
    printf("Padding = %i\n", st->padding);
    printf("============================================================================================================\n");
}


void print_functionTable(FunctionTable *ft)
{
    printf("=================================================================\n");
    printf("%-16s %-20s %-8s %-8s %-8s\n", "Name", "Param Count", "Index", "Scope", "Params");
    printf("-----------------------------------------------------------------\n");

    for (int i = 0; i < ft->count; i++)
    {
                
        printf("%-16s %-20i %-8i %-8i",
               ft->functions[i]->fmi->name,
               ft->functions[i]->fmi->param_count,
               ft->functions[i]->fmi->index,
               ft->functions[i]->fmi->scope);

        printf("\n");
    }

    printf("=================================================================\n");
}














