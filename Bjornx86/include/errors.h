

#define ERROR_MANAGER_INIT_CAPACITY 10
#define ERROR_MANAGER_RESIZE_FACTOR 2

#define ERROR_MSG_INIT_CAPACITY 10
#define ERROR_MSG_RESIZE_FACTOR 2

#define MAX_ERRORS_TILL_EXIT 5


typedef enum ErrorID
{
    /** During ast */
    FILE_NOT_FOUND,
    EXPECTED_TOKEN_NOT_MET,
    /** During analysis */
    NESTING_OVERFLOW,
    CONTEXT_NOT_ALLOWED,
    BINARYOP_NOT_TRIVIAL,
    TYPE_UNDEFINED,
    TYPE_INCOMPLETE,
    ARRAY_SIZE_MISMATCH,
    ARRAY_TYPE_MIXING,
    RTYPE_CANT_BE_ASSIGNED_TO_LTYPE,
    FIELD_NOT_DEFINED_IN_CLASS,
    FIELD_NOT_DEFINED_IN_OBJECT,
    ENUM_DOESNT_HAVE_FIELDS,
    VARIABLE_DEFINITION_NOT_FOUND,
    TYPE_NOT_SUBSCRIPTABLE,
    REDEFINITION_OF_VARIABLE,
    REDEFINITION_OF_UDT,
    EXTERN_FUNC_DEF_ONLY_ALLOWED_GLOBALLY,
    RETURN_TYPE_IS_ARRAY,
    RETURN_TYPE_BIGGER_8B,
    PARAM_TYPE_IS_ARRAY,
    PARAM_TYPE_BIGGER_8B,
    ACTUAL_RETTYPE_CANT_BE_ASSIGNED_TO_DECLARED_RETTYPE,
    UNARYOP_INVALID,
    REDEFINITION_OF_FUNCTION,
    VARARG_FUNC_MUST_HAVE_UNIQUE_NAME,
    VARARG_MUST_BE_LAST_PARAM,
    EXPECTED_TYPE_MUST_BE_NUMBER,
    TYPE_UNEXPECTED,
    FUNCTION_NOT_FOUND,
    GLOBAL_ELEMENT_NOT_ALLOWED,
    INVALID_LVALUE,
    AMBIGUITY_DURING_FUNCCALL,

}ErrorID;

typedef enum ErrorSeverity
{
    ERROR_WARNING,
    ERROR_FATAL,
}ErrorSeverity;


typedef struct Error{
    ErrorID error_id;
    ErrorSeverity severity;
    char *src_file;
    char **messages;
    int line_number;
    int message_count;
    int message_capacity;
}Error;

typedef struct ErrorManager{
    Error **errors;
    int errors_count;
    int errors_capacity;
}ErrorManager;


extern ErrorManager *errorManager;

void __initErrorManager();
void errorManagerCheckInBeforeBuild();
char *fetchErrorMsgBasedOnErrorID(ErrorID e_id);
void appendMessageToError(Error *e, char *fmt, ...);
Error *invoke(ErrorID e_id);
void printErrors();
void printError(Error *e);
void addError(Error *e);
ErrorSeverity getSeverityOfErrorID(ErrorID e_id);
