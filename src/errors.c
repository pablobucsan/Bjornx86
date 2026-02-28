#include "../include/errors.h"
#include "../include/tokenizer.h"
#include "../include/types.h"
#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>

// have enter_node update the currrent line

ErrorManager *errorManager = NULL;

void __initErrorManager()
{
    errorManager = malloc(sizeof(ErrorManager));
    errorManager->errors = malloc(sizeof(Error *) * ERROR_MANAGER_INIT_CAPACITY);
    errorManager->errors_capacity = ERROR_MANAGER_INIT_CAPACITY;
    errorManager->errors_count = 0;
}

void __resizeErrorManager()
{
    errorManager->errors_capacity *= ERROR_MANAGER_RESIZE_FACTOR;
    errorManager->errors = realloc(errorManager->errors, errorManager->errors_capacity * sizeof(Error *) );
}

void addError(Error *e)
{
    if (errorManager->errors_count + 1 >= errorManager->errors_capacity){
        __resizeErrorManager();
    }



    errorManager->errors[errorManager->errors_count++] = e;
    /** If it's a fatal error, stop compiling */
    if (e->severity == ERROR_FATAL){
        printErrors();
        exit(1);
    }

    /** If too many errors, print them and stop compiling */
    if (errorManager->errors_count >= MAX_ERRORS_TILL_EXIT){
        printErrors();
        exit(1);
    }
}

void printErrors()
{
    for (int i = 0; i < errorManager->errors_count; i++){
        printError(errorManager->errors[i]);
    }
}

void __resizeErrorMessages(Error *e)
{
    e->message_capacity *= ERROR_MSG_RESIZE_FACTOR;
    e->messages = realloc(e->messages, sizeof(char *) * e->message_capacity);
}

Error *invoke(ErrorID e_id)
{
    Error *e = malloc(sizeof(Error));
    e->error_id = e_id;
    e->severity = getSeverityOfErrorID(e_id);
    e->line_number = tracker.current_line;
    e->src_file = tracker.current_src_file;
    e->message_capacity = ERROR_MSG_INIT_CAPACITY;
    e->message_count = 0;
    e->messages = malloc(sizeof(char *) * e->message_capacity);

    return e;
}

void appendMessageToError(Error *e, char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);

    /** Determine the required buffer size */
    va_list args_copy;
    va_copy(args_copy, args);
    int size = vsnprintf(NULL,0, fmt, args_copy); 
    va_end(args_copy);

    char *buffer = malloc(size + 1);
    vsnprintf(buffer, size + 1, fmt, args);
    va_end(args);

    if (e->message_count + 1 >= e->message_capacity){
        __resizeErrorMessages(e);
    }
    e->messages[e->message_count++] = buffer;
}

char *fetchErrorMsgBasedOnErrorID(ErrorID e_id)
{
    switch(e_id)
    {
        /** During AST */
        case FILE_NOT_FOUND:{
            return "File: '%s' not found.";
        }
        case NUMBER_TOO_BIG:{
            return "Number is too big, will be truncated to 8 bytes.";
        }
        case EXPECTED_TOKEN_NOT_MET:{
            return "Missing: '%s'. Instead got: '%s'.";
        }
        /** During analysis */
        case NESTING_OVERFLOW:{
            return "Cannot push another context: '%s', max nesting is: %i.";
        }
        case CONTEXT_NOT_ALLOWED:{
            return "Cannot push the context: '%s' within context of type: '%s'.";
        }
        case BINARYOP_NOT_TRIVIAL:{
            return "Binary operation: '%s' between types: '%s' and '%s' is not trivial.";
        }
        case TYPE_UNDEFINED:{
            return "Type: '%s' is undefined.";
        }
        case TYPE_INCOMPLETE:{
            return "Type: '%s' is incomplete for usage.";
        }
        case RTYPE_CANT_BE_ASSIGNED_TO_LTYPE:{
            return "LVALUE with type: '%s' can't be assigned RVALUE with type: '%s'";
        }
        case FIELD_NOT_DEFINED_IN_CLASS:{
            return "Field: '%s' is not defined in class '%s'.";
        }
        case FIELD_NOT_DEFINED_IN_OBJECT:{
            return "Field: '%s' is not defined in object '%s'.";
        }
        case ENUM_DOESNT_HAVE_FIELDS:{
            return "User Defined Type: '(ENUM): %s', does not have fields.";
        }
        case VARIABLE_DEFINITION_NOT_FOUND:{
            return "Variable definition of: '%s', not found in current scope nor in any parent scope.";
        }
        case TYPE_NOT_SUBSCRIPTABLE:{
            return "Type: '%s', is not subscriptable.";
        }
        case REDEFINITION_OF_VARIABLE:{
            return "Redefinition of variable with name: '%s'.";
        }
        case REDEFINITION_OF_UDT:{
            return "Redefinition of UDT with name: '%s'.";
        }
        case EXTERN_FUNC_DEF_ONLY_ALLOWED_GLOBALLY:{
            return "Extern function definition with name: '%s', is only allowed in global context.";
        }
        case EXTERN_ID_DEF_ONLY_ALLOWED_GLOBALLY:{
            return "Extern variable definition with name: '%s', is only allowed in global context.";
        }
        case RETURN_TYPE_IS_ARRAY:{
            return "In function: '%s', return type is of type array which is not allowed. Consider using a pointer instead.";
        }
        case RETURN_TYPE_BIGGER_8B:{
            return "In function: '%s', return type: '%s', is bigger than 8 bytes which is not allowed. Consider using a pointer instead.";
        }
        case PARAM_TYPE_IS_ARRAY:{
            return "In function: '%s', parameter: '%s', is of type array which is not allowed. Consider using a pointer instead.";
        }
        case PARAM_TYPE_BIGGER_8B:{
            return "In function: '%s', parameter: '%s', with type: '%s', is bigger than 8 bytes which is not allowed. Consider using a pointer instead.";
        }
        case ACTUAL_RETTYPE_CANT_BE_ASSIGNED_TO_DECLARED_RETTYPE:{
            return "In function: '%s' declared a return type: '%s', but actual return type is: '%s', which can't be assigned to declared.";
        }
        case UNARYOP_INVALID:{
            return "Invalid operand type: '%s' to apply unary op to: '%s'.";
        }
        case REDEFINITION_OF_FUNCTION:{
            return "Redefinition of function with name: '%s'.";
        }
        case EXPECTED_TYPE_MUST_BE_NUMBER:{
            return "Expected type must be a number, but got: '%s'.";
        }
        case FUNCTION_NOT_FOUND:{
            return "In function call: No function with name: '%s' was found.";
        }
        case TYPE_UNEXPECTED:{
            return "Got type: '%s'.";
        }
        case GLOBAL_ELEMENT_NOT_ALLOWED:{
            return "Expression must be able to evaluate at compile time.";
        }
        case ARRAY_SIZE_MISMATCH:{
            return "Array size mismatch.";
        }
        case ARRAY_TYPE_MIXING:{
            return "Array init element type mixing.";
        }
        case INVALID_LVALUE:{
            return "Invalid LVALUE to reassign: '%s'.";
        }
        case AMBIGUITY_DURING_FUNCCALL:{
            return "Ambiguity during function call with name: '%s'.";
        }
        case VARARG_FUNC_MUST_HAVE_UNIQUE_NAME:{
            return "Function with vararg parameters must have unique name.";
        }
        case VARARG_MUST_BE_LAST_PARAM:{
            return "Varargs must be the last parameter for function: '%s'.";
        }
        case FIELD_WITH_INCOMPLETE_TYPE:{
            return "Field: '%s', has incomplete type: '%s'.";
        }
        case CONTINUE_NOT_IN_LOOP:{
            return "Keyword: 'continue', only makes sense within a loop scope.";
        }
        case BREAK_NOT_IN_LOOP:{
            return "Keyword: 'break', only makes sense within a loop scope.";
        }
        case CLASS_UNDEFINED:{
            return "Class: '%s' is undefined.";
        }
        case ILL_FORMATED_ENUM:{
            return "Ill formated enum. Correct usage is \"IDENTIFIER\" | \"IDENTIFIER = NUMBER\".";
        }
        default:{
            fprintf(stderr, "Unknown error id to get error msg: %i\n", e_id);
            exit(1);
        }
    }

    return NULL;
}

char *getSeverityLabelFromErrorSeverity(ErrorSeverity e_severity)
{
    switch(e_severity){
        case ERROR_FATAL: return "ERROR FATAL";
        case ERROR_WARNING: return "ERROR WARNING";
        default:{
            fprintf(stderr, "Unknown severity to get label from: %i\n", e_severity);
            exit(1);
        }
    }
}

ErrorSeverity getSeverityOfErrorID(ErrorID e_id)
{
    switch(e_id)
    {
        case CONTEXT_NOT_ALLOWED:
        case NESTING_OVERFLOW:
        case TYPE_UNDEFINED:
        case FIELD_NOT_DEFINED_IN_CLASS:
        case FIELD_NOT_DEFINED_IN_OBJECT:
        case ENUM_DOESNT_HAVE_FIELDS:
        case VARIABLE_DEFINITION_NOT_FOUND:
        case FUNCTION_NOT_FOUND:
        case VARARG_FUNC_MUST_HAVE_UNIQUE_NAME:
        case VARARG_MUST_BE_LAST_PARAM:
        case TYPE_NOT_SUBSCRIPTABLE:
        case REDEFINITION_OF_UDT:
        case REDEFINITION_OF_VARIABLE:
        case UNARYOP_INVALID:
        case TYPE_UNEXPECTED:
        case ARRAY_SIZE_MISMATCH:
        case ARRAY_TYPE_MIXING:
        case INVALID_LVALUE:
        case REDEFINITION_OF_FUNCTION:
        case AMBIGUITY_DURING_FUNCCALL:
        case FIELD_WITH_INCOMPLETE_TYPE:
        case BREAK_NOT_IN_LOOP:
        case CONTINUE_NOT_IN_LOOP:
        case CLASS_UNDEFINED:
        case ILL_FORMATED_ENUM:
        case FILE_NOT_FOUND: return ERROR_FATAL;

        case TYPE_INCOMPLETE: 
        case BINARYOP_NOT_TRIVIAL:
        case EXTERN_FUNC_DEF_ONLY_ALLOWED_GLOBALLY:
        case EXTERN_ID_DEF_ONLY_ALLOWED_GLOBALLY:
        case RTYPE_CANT_BE_ASSIGNED_TO_LTYPE:
        case RETURN_TYPE_IS_ARRAY:
        case RETURN_TYPE_BIGGER_8B:
        case PARAM_TYPE_IS_ARRAY:
        case PARAM_TYPE_BIGGER_8B:
        case GLOBAL_ELEMENT_NOT_ALLOWED:
        case EXPECTED_TYPE_MUST_BE_NUMBER:
        case ACTUAL_RETTYPE_CANT_BE_ASSIGNED_TO_DECLARED_RETTYPE:
        case NUMBER_TOO_BIG:
        case EXPECTED_TOKEN_NOT_MET: return ERROR_WARNING;
        default:{
            fprintf(stderr, "Unknown error id to get severity of: %i\n", e_id);
            exit(1);
        }
    }
}

void printError(Error *e)
{
    printf("[%s]:", getSeverityLabelFromErrorSeverity(e->severity));
    printf("In file: '%s'. ", e->src_file);
    printf("In L = %i. ", e->line_number);
    for (int i = 0; i < e->message_count; i++){
        printf("%s",e->messages[i]);
    }
    printf("\n\n");
}


void errorManagerCheckInBeforeBuild()
{
    /** If we have any errors, we can not proceed to build */
    if (errorManager->errors_count > 0){
        printErrors();
        exit(1);
    }
}
