

#include "ast.h"
#include "analyzer.h"
#include "stdio.h"
#include "tables.h"


#define MAX_PI_COUNT 10
#define MAX_EXPECTATIONS 10
#define MAX_WORKING_CONTEXT 10
#define MAX_SPILLS 64

typedef struct Register Register;

typedef enum RegisterType 
{
    GPR, 
    FPR,
    ANY
}RegisterType;

// Operands can be 
typedef struct Operand 
{   
    Register *linkedToRegister;
    int is_unsigned;
}Operand;


typedef struct CurrentFrameAndFunction 
{
    int stack_base_ptr;
    Function *f;
}CurrentFrameAndFunction;

typedef enum WCType 
{
    COMPUTING_IMMEDIATE,
    COMPUTING_LVALUE,
    COMPUTING_RVALUE,
}WCType;

typedef struct WorkingContext 
{
    WCType typeStack[MAX_WORKING_CONTEXT];
    int currentContextPtr;
}WorkingContext;





/**
 * 
 * Everytime we spill a register, log it with the RequestOffice, who will 
 * create a SpillRecord so later on we can restore spilled registers in correct order
 */
typedef struct SpillRecord
{
    int is_restored;   // whether its been restored
    Register *reg;     // The 64 bit parent that's been pushed
}SpillRecord;

typedef struct RequestOffice
{
    int spill_count;
    SpillRecord spills[MAX_SPILLS];
}RequestOffice;

typedef struct RegisterTable 
{
    Register **registers;
    int num_of_registers;
    
}RegisterTable;


typedef struct Register 
{
    char *name;
    int size;               // size of the register in bytes

    int is_pinned;          // its value cannot be changed because it hasnt been consumed, if we need a pinned register, we have to spill it
    int is_spilled;         // has it been pushed to the stack?

    Register *parentRegister; // direct parent al->ax->eax->rax
    Register *parent64Register;  // al->rax; 
    Register *subRegisters[3];   // eax, ax, al

    RegisterType type; //fpr,gpr

}Register;




/*———————————————————————————————————  EXPECT - MATCH —————————————————————————————————————————————————————————  */
/**
 * 
 * TRACK ALSO MAYBE THE CONTEXT WHO PUSHED THE EXPECTATION (AND ID) SO WE USE THE SAME EXPECTATION FOR THE WHOLE NODE?
 */
typedef struct Expectation
{
    ContextType ctx_type;                   // Who pushed this expectation? NODE_ASSIGNMENT, NODE_REASSIGNMENT, ETC?
    RegisterType registerType; 
    int size;
}Expectation;


typedef struct Matcher 
{
    Expectation **expecations_to_match;
    int current_expectation_index;
}Matcher;

/*———————————————————————————————————  EXPECT - MATCH —————————————————————————————————————————————————————————  */





/**
 * 
 * 28/11 Absolutely change this
 */
// 26/11: Have modification track arguments pushed and registers pushed?
typedef struct Modification
{
    int n;
    int has_been_modified;
}Modification;

typedef struct RSPTracker
{
    Modification modifications_stack[MAX_NESTING];              // able to do 10 start_tracking_rsp() without ever doing stop_tracking()
    int index;
    int is_active;
}RSPTracker;



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
    BIT_AND,
    BIT_OR,
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

typedef enum PostfixOPCode
{
    POSTFIX_INC,
    POSTFIX_DEC
}PostfixOPCode;




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


// Working Context 
extern WorkingContext workingContext;

void initWorkingContext();
WCType getCurrentWorkingContext();

Operand with_lvalue_context(FILE *asm_file, ASTNode *node, SymbolTable *st, FunctionTable *ft);
Operand with_rvalue_context(FILE *asm_file, ASTNode *node, RegisterType rtype, int size, SymbolTable *st, FunctionTable *ft);

void push_bt_context(ContextType ctx_type);
void pop_bt_context();

void initMatcher();
Register *matchExpectedRegister();
void expectRegister(RegisterType registerType, int size);
Register *getParent64(Register *reg);
char *getWordForSize(int size);


void pushWorkingContext(WCType type);
void popWorkingContext();
void popAllWorkingContexts();

ContextType astToContext(ASTNode *node);
int compute_binaryOperationDepth(ASTNode *node, int depth);

void emit_push(FILE *asm_file, char *reg_name);
void emit_pop(FILE *asm_file, char *reg_name);
void emit_comment(FILE *asm_file, char *comment);

int align_rsp(int locals_weight);
void initMatcher();
Expectation *getCurrentExpectation();
Register *matchExpectedRegister();
void expectRegister(RegisterType registerType, int size);



void clearExpectation();
void clearAllExpectations();

void init_GPR();
void init_FPR();
Register *populateFPR(int index, char *name, int caller_saved, int callee_saved);
Register *getGPR(int size);
Register *getFPR(int size);
Register *getUnspilledGPR(int size);
int getStackBasePtr();

void clearRegisters(RegisterTable *table);
void unpinRegister(Register *reg);

void collectStrings(FILE *asm_file, ASTNode *program);
void buildInnerFunc(FILE *asm_file, ASTNode *program, SymbolTable *st, FunctionTable *ft);
void buildData(FILE *asm_file, ASTNode *program, SymbolTable *current_st, FunctionTable *current_ft);
Operand buildStart(FILE *asm_file, ASTNode *program, SymbolTable *current_st, FunctionTable *current_ft);
Operand buildExtern(FILE *asm_file, ASTNode *program, FunctionTable *gb_functionTable);
