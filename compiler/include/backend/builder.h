

#include "../frontend/ast.h"
#include "analyzer.h"
#include "stdio.h"
#include "tables.h"


#define MAX_PI_COUNT 10
#define MAX_EXPECTATIONS 10
#define MAX_WORKING_CONTEXT 10
#define MAX_SPILLS 64



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
 * Represents the spilling done by a node that evaluates to a value
 */
typedef struct SpillReport
{
    ASTNode *ast_responsible;
    int reg_spilled_count;
    Register *spilled_regs[MAX_SPILLS];
}SpillReport;

/**
 * Represents all the spill reports
 */
typedef struct SpillReportDrawer
{
    int report_count;
    SpillReport reports[MAX_NESTING];
}SpillReportDrawer;

extern SpillReportDrawer SRD;

typedef struct RegisterTable 
{
    Register **registers;
    int num_of_registers;
}RegisterTable;







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
Function *getCurrentFunction();

Operand *with_lvalue_context(FILE *asm_file, ASTNode *node, SymbolTable *st, FunctionTable *ft);
Operand *with_rvalue_context(FILE *asm_file, ASTNode *node, RegisterType rtype, int size, SymbolTable *st, FunctionTable *ft);

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
void emit_mov_label2r(FILE *asm_file, Register *dst, char *label);
void emit_mov_derefLabel2r(FILE *asm_file, Register *dst, char *label);
void emit_mov_imm2r(FILE *asm_file, Register *dst, uint64_t n);
void emit_mov_r2r(FILE *asm_file, Register *dst, Register *src, int bytes_to_move, int is_unsigned);
void emit_mov_s2r(FILE *asm_file, Register *dst, int offset, int bytes_to_read, int is_unsigned);
char *getLeaOrMovBasedOnContext();

Operand *load_imm(FILE *asm_file, size_t size, ASTNode *imm_node);
Operand *load_identifier(FILE *asm_file, Symbol *s);

int align_rsp(int locals_weight);
void initMatcher();
Expectation *getCurrentExpectation();
Register *matchExpectedRegister();
void expectRegister(RegisterType registerType, int size);
Register *request_unspilled_specific_size_reg(int size);
Register *request_unspilled_ssr(int size);

void clearExpectation();
void clearAllExpectations();
void initGlobalOperand();
void init_GPR();
void init_FPR();
Register *populateFPR(int index, char *name, int caller_saved, int callee_saved);
Register *getGPR(FILE *asm_file, int size);
Register *getFPR(int size);
Register *getUnspilledGPR(int size);
Register *getFamilyRegWithSize(Register *reg, int size);
int nextPowerOfTwo(int given_size);
int isPowerOfTwo(int size);
int are_reg_family(Register *a, Register *b);
int getStackBasePtr();

void clearRegisters(RegisterTable *table);
void unpinRegister(Register *reg);
void pinRegister(Register *reg);
int is_in_spill_report(Register *reg, SpillReport *report);
Register **getPinnedGPRs(int *in_count);
void spill_reg(FILE *asm_file, Register *reg);

void collectStrings(FILE *asm_file, ASTNode *program);
void buildInnerFunc(FILE *asm_file, ASTNode *program, SymbolTable *st, FunctionTable *ft);
void buildData(FILE *asm_file, ASTNode *program, SymbolTable *current_st, FunctionTable *current_ft);
Operand *buildStart(FILE *asm_file, ASTNode *program, SymbolTable *current_st, FunctionTable *current_ft);
void buildExtern(FILE *asm_file, ASTNode *program, FunctionTable *gb_functionTable);
