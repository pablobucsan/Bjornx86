#ifndef ABC_TYPES_H
#define ABC_TYPES_H


/**
 * ===============================================================================
 * TYPES DEFINITION      
 * ===============================================================================   
 */

typedef struct Type Type;
typedef struct TypeTable TypeTable;
typedef struct SymbolTable SymbolTable;
typedef struct FunctionTable FunctionTable;

#define TYPETABLE_INIT_CAPACITY 10
#define TYPETABLE_RESIZE_FACTOR 2

extern TypeTable *gb_typeTable;

typedef struct TypeTable
{
    Type **types;
    int capacity;
    int count;
    int undef_count;
}TypeTable;

typedef enum TypeKind
{
    TYPE_VOID,
    TYPE_NUMBER, // uintX, intX, bool, char
    TYPE_STR,
    TYPE_PTR,
    TYPE_ARRAY,
    TYPE_UDT,    // user defined type
}TypeKind;

typedef enum UserDefinedTypeKind
{
    UDT_UNKNOWN_YET,
    UDT_ENUM,
    UDT_OBJECT,
    UDT_CLASS
}UserDefinedTypeKind;

typedef enum NumberKind
{
    NUMBER_CHAR,
    NUMBER_BOOL,
    NUMBER_INTEGER
}NumberKind;

typedef enum ArraySizeKind
{
    ARR_SIZE_GIVEN,
    ARR_SIZE_INTERPRETED
}ArraySizeKind;
// Encapsulates ints, uints, char and bools
typedef struct NumberType
{
    NumberKind kind;
    int bytes;
    int is_unsigned;
}NumberType;

typedef struct StringType
{

}StringType;

typedef struct PointerType
{
    Type *pointed_type;
}PointerType;

typedef struct ArrayType
{
    ArraySizeKind size_provided_kind;
    int n_of_elements;
    Type *element_type;
}ArrayType;


typedef struct UserDefinedType
{
    UserDefinedTypeKind udt_kind;
    char *name;
}UserDefinedType;



typedef struct Type
{
    TypeKind kind;
    union {
        NumberType numberType;
        StringType stringType;
        PointerType pointerType;
        ArrayType arrayType;
        UserDefinedType udt;
    };
    char *representation;

}Type;

char *typeToString(Type *type);

#endif //ABC_TYPES_H
