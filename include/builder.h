

#include "ast.h"
#include "analyzer.h"
#include "stdio.h"
#include "tables.h"


#define MAX_PI_COUNT 10

typedef struct Register Register;



typedef struct Register 
{
    char *name;
    int is_being_used;      // is it being used 
    int size;               // bytes
    
    Register *parentRegister; // direct parent al->ax->eax->rax
    Register *parent64Register;  // al->rax; 
    Register *subRegisters[3];   // eax, ax, al

}Register;


typedef struct RegisterTable 
{
    Register **registers;
    int num_of_registers;
    
}RegisterTable;

// Operands can be 
// 1. Immediate: 3
// 2. Registers: rax, eax, ax, al  
// 3. Pointers: [rax] -> value at the address sitting in rax 
// 4. Pointers with offsets: [rax - 8]
// 5. Label
typedef struct Operand 
{
    enum 
    {
        NUMBER, 
        IDENTIFIER,
        LABEL,
        NONE
    }type;
    
    Register *linkedToRegister;
    char *word;
}Operand;


// Expect - Match  
typedef enum RegisterType 
{
    GPR, 
    FPR,
    ANY
}RegisterType;


typedef struct Expectation
{
    RegisterType registerType; 
    int size;
}Expectation;



typedef struct Matcher 
{
    Expectation *expecation_to_match;
    int empty;
}Matcher;




typedef enum BinOPCode
{
    ADD,
    MUL,
    DIV,
    SUB,
    MOD,
    EQ,
    NEQ,
    LT,
    GT,
    LTEQ,
    GTEQ,
    AND,
    OR,
    UNKNOWN
}BinOPCode;


typedef enum UnaryOPCode
{
    UNARY_MINUS,
    UNARY_DEREF,
    UNARY_NOT,
    UNARY_ADDRESS

}UnaryOPCode;




/*
typedef struct Instruction
{
OPCODE opcode;
int arg[3];
}Instruction;
*/

typedef enum PatchInstGenerators
{
SKIP = 1,
STOP = 2,
}PatchInstGenerators;

typedef struct PatcheableInstruction
{
PatchInstGenerators generator;      //who generated the patcheable instruction 
ContextType ctx;                    //context in which this instruction was generated 
int address;                        //address of the Instruction to patch 
}PatcheableInstruction;


typedef struct PatchInstructManager
{
PatcheableInstruction pi_array[MAX_PI_COUNT];        // patcheable instruction array
int pip;                                            //patcheable instruction pointer
}PatchInstructManager;



// General Purpose Register 
extern RegisterTable* gpr;
// Function Parameter Register
extern RegisterTable* fpr;


// Matcher 
extern Matcher *matcher;





void initMatcher();
Register *matchExpectedRegister();
void expectateRegister(RegisterType registerType, int size);


void init_GPR();
void init_FPR();
Register *populateFPR(int index, char *name);
Register *getGPR(int size);
Register *getFPR(int size);


Operand buildData(FILE *asm_file, ASTNode *program, SymbolTable *current_st);
Operand buildStart(FILE *asm_file, ASTNode *program, SymbolTable *current_st, FunctionTable *current_ft);
Operand buildExtern(FILE *asm_file, ASTNode *program, FunctionTable *gb_functionTable);
