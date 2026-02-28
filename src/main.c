#include "../include/tokenizer.h"
#include "../include/ast.h"
#include "../include/analyzer.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../include/builder.h"
#include <unistd.h>  // for getcwd
#include <limits.h>  // for PATH_MAX
#include <time.h>

void genExit(FILE *file);


typedef struct CompilerFlags
{
    int print_ast;
    int print_tokens;
    int only_assemble;
    int debug;
    int no_link;
    int time;
}CompilerFlags;

typedef struct CompilerArgs
{
    char *scriptPath;
    char *outputPath;
}CompilerArgs;

typedef struct CompilerTerminators
{
    int help;
}CompilerTerminators;


typedef struct CompilerOptions
{
    CompilerFlags *CF;
    CompilerArgs *CA;
    CompilerTerminators *CT;
}CompilerOptions;



void genExit(FILE *file)
{
    fprintf(file, "\n    ;Exit execution\n\n");
    // 60 for signaling we want to exit 
    fprintf(file, "    mov rax, 60\n");
    // 0  for error code 
    fprintf(file, "    mov rdi, 0\n");
    // syscall to exit 
    fprintf(file, "    syscall\n\n");
}


void genDataStdFileDescriptors(FILE *file)
{
    fprintf(file, "STDOUT_struct:\n");
    fprintf(file, "    dq 0\n");            // stdout->path
    fprintf(file, "    dq 0\n");            // stdout->fpos
    fprintf(file, "    dd 1\n");            // stdout->fd
    fprintf(file, "    dd 1\n");            // stdout->open_flags
    fprintf(file, "    dd 0\n");            // stdout->mode

    fprintf(file, "STDOUT:\n");
    fprintf(file, "    dq STDOUT_struct\n");

}
void genData(FILE *file, ASTNode *program)
{
    genDataStdFileDescriptors(file);
    buildData(file, program, gb_symbolTable, gb_functionTable);
    fprintf(file,"\n\n\n");
}

void genExtern(FILE *file, ASTNode *program)
{
    fprintf(file, "\n; Extern functions \n");
    buildExtern(file, program, gb_functionTable);
}



void genRunTimeInit(FILE *file)
{
    fprintf(file,"\n");
    fprintf(file, "global _start\n");
    fprintf(file, "global _main_u64_ppc\n");
    fprintf(file, "_start:\n");
    fprintf(file, "    mov rbp, qword rsp\n");
    fprintf(file, "    mov rdi, qword [rbp]\n");
    fprintf(file, "    lea rsi, qword [rbp + 8]\n");
    fprintf(file, "    call _main_u64_ppc\n");
    genExit(file);
}


void genStart(FILE *file, ASTNode *program)
{

    genRunTimeInit(file);
    initMatcher();
    init_GPR();
    init_FPR();
    buildStart(file, program, gb_symbolTable, gb_functionTable);

}


void compile_tokenize(CompilerOptions *CO, Token ***tokens, int *token_pos)
{
    initTokenizerContext();
    // TOKENIZE THE SOURCE CODE 
    char *src = read_file(CO->CA->scriptPath);
    *tokens = tokenize(src, CO->CA->scriptPath);
    *token_pos = 0; 
    
    if (CO->CF->print_tokens) { print_tokens(*tokens); }
}

ASTNode *compile_parse(CompilerOptions *CO, Token **tokens, int *token_pos)
{
    initASTContext();

    ASTNode *program = NULL;
    if (CO->CF->debug) { 
        printf("==== >>>>> ABOUT TO PARSE  ====\n");
        program = parseProgram(tokens, token_pos);
        printf("====    ✅ DONE PARSING ==== \n");
    }
    else{
        program = parseProgram(tokens, token_pos);
    }
    if (CO->CF->print_ast) { 
        print_ast(program,0); 
    }
    return program;
}

void compile_analyze(CompilerOptions *CO, ASTNode *program)
{

    initAnalysisContext();
    initGlobalTables();
    if (CO->CF->debug){
        printTypeTable();
        printf("==== >>>>> ABOUT TO ANALYZE ====\n");
        analyze(program, gb_symbolTable, gb_functionTable); 
        printf("====    ✅ DONE ANALYZING ====\n");
        printTypeTable();
    }
    else{
        analyze(program, gb_symbolTable, gb_functionTable);
    }

    resetContext();
    errorManagerCheckInBeforeBuild();
}

FILE *compile_build(CompilerOptions *CO, ASTNode *program)
{
    char *file = strdup(CO->CA->outputPath);
    char *buffer = ".asm\0";

    char *output_filePath = strcat(file, buffer);
    FILE *asm_file = fopen(output_filePath, "w");
    if (asm_file == NULL){
        fprintf(stderr, "Path: '%s', not found.\n", file);
        exit(1);
    }

    // DATA SECTION
    fprintf(asm_file, "section .data\n");
    // BUILD PHASE 1: COLLECT STRINGS
    if (CO->CF->debug){
        printf("==== >>>>> ABOUT TO COLLECT STRINGS ====\n");
        collectStrings(asm_file, program);
        printf("====    ✅ DONE COLLECTING STRINGS ====\n");
    }
    else{
        collectStrings(asm_file, program);
    }

    // BUILD PHASE 2: GLOBAL DATA
    if (CO->CF->debug){
        printf("==== >>>>> ABOUT TO BUILD DATA ====\n");
        genData(asm_file, program);
        printf("====    ✅ DONE BUILDING DATA ====\n");
    }
    else{
        genData(asm_file, program);
    }

    resetContext();


    // TEXT SECTION 
    fprintf(asm_file, "section .text\n");
    // BUILD PHASE 3: EXTERN LABELS
    if (CO->CF->debug){
        printf("==== >>>>> ABOUT TO BUILD EXTERN ====\n");
        genExtern(asm_file, program);
        printf("====    ✅ DONE BUILDING EXTERN ====\n");
    }
    else{
        genExtern(asm_file, program);
    }

    resetContext();
    
    // BUILD PHASE 4: START
    if (CO->CF->debug){
        printf("==== >>>>> ABOUT TO BUILD START ====\n");
        genStart(asm_file, program);
        printf("====   ✅ DONE BUILDING START ====\n");
    }
    else{
        genStart(asm_file, program);
    }

    return asm_file;
}

void compile_produce_asm(CompilerOptions *CO)
{
    // At this point we have the asm
    if (CO->CF->only_assemble){
        printf("File: '%s', successfully generated\n", CO->CA->outputPath);
        exit(1);
    }
}

void compile_produce_ofiles(CompilerOptions *CO)
{
    char *file = strdup(CO->CA->outputPath);
    char *buffer = ".asm\0";

    char *output_filePath = strcat(file, buffer);

    char obj_file[PATH_MAX];
    sprintf(obj_file, "%s.o", CO->CA->outputPath);


    char cmd[512];
    sprintf(cmd, "nasm -f elf64 %s -o %s", output_filePath , obj_file);
    int status = system(cmd);
    if (status != 0){
        fprintf(stderr, "nasm failed: %s\n", cmd);
        exit(1);
    }
    if (CO->CF->debug){
        printf("%s\n", cmd);
    }
        

}

void compile_link(CompilerOptions *CO)
{
    if (CO->CF->no_link){
        return;
    }
    // Link
    char exe_file[PATH_MAX];
    sprintf(exe_file, "%s", CO->CA->outputPath);  // no extension by default

    char obj_file[PATH_MAX];
    sprintf(obj_file, "%s.o", CO->CA->outputPath);
    char cmd[512];

    char lib_obj[PATH_MAX];
    sprintf(lib_obj, "%s/bjorn-std/obj/*.o", getenv("HOME"));

    // Link with your user .o libs
    sprintf(cmd, "ld -o %s %s %s",
            exe_file, obj_file, lib_obj); // adjust path
    
    int status = system(cmd);
    if (status != 0) {
        fprintf(stderr, "ld failed: %s\n", cmd);
        exit(1);
    }
    if (CO->CF->debug){
        printf("%s\n", cmd);
        printf("Executable written to %s\n", exe_file);
    }

}

void compile(CompilerOptions *CO)
{
    clock_t start,end;
    double time_taken;


    // Ensure input and output paths were given 
    if (CO->CA->scriptPath == NULL){
        fprintf(stderr, "No input source file was specified. Run 'bjornc --help' for instructions on usage\n");
        exit(1);
    }

    if (CO->CA->outputPath == NULL){
        fprintf(stderr, "No output destination file was specified. Run 'bjornc --help' for instructions on usage.\n");
        exit(1);
    }


    // STAGE 1: TOKENIZE   
    Token **tokens = NULL;
    int token_pos = 0; 

    if (CO->CF->time){
        start = clock();
        compile_tokenize(CO, &tokens, &token_pos);
        end = clock();
        time_taken = ((double)(end - start)) / CLOCKS_PER_SEC * 1000;
        printf("[TIMING] Lexing: %.2f ms\n", time_taken);
    }
    else{
        compile_tokenize(CO, &tokens, &token_pos);
    }

    ASTNode *program = NULL;
    // STAGE 2: AST GEN
    if (CO->CF->time){
        start = clock();
        program = compile_parse(CO, tokens, &token_pos);
        end = clock();
        time_taken = ((double)(end - start)) / CLOCKS_PER_SEC * 1000;
        printf("[TIMING] Parsing: %.2f ms\n", time_taken);
    }
    else{
        program = compile_parse(CO, tokens, &token_pos);
    }

    // STAGE 3: ANALYZE 
    if (CO->CF->time){
        start = clock();
        compile_analyze(CO, program);
        end = clock();
        time_taken = ((double)(end - start)) / CLOCKS_PER_SEC * 1000;
        printf("[TIMING] Analyzing: %.2f ms\n", time_taken);
    }
    else{
        compile_analyze(CO, program);
    }

    FILE *asm_file = NULL;
    // STAGE 4: BUILD
    if (CO->CF->time){
        start = clock();
        asm_file = compile_build(CO, program);
        end = clock();
        time_taken = ((double)(end - start)) / CLOCKS_PER_SEC * 1000;
        printf("[TIMING] Building: %.2f ms\n", time_taken);
    }
    else{
        asm_file = compile_build(CO, program);
    }
    // Close the file. IMPORTANT ***BEFORE*** DOING ANYTHING WITH SYSTEM
    fflush(asm_file);
    fclose(asm_file);

    // STAGE 5: ASSEMBLE AND LINK. ASM->O->EXE
    // STAGE 5 PHASE 1: ASSEMBLE
    compile_produce_asm(CO);
    // STAGE 5 PHASE 2: PRODUCE O FILES
    compile_produce_ofiles(CO);
    // STAGE 5 PHASE 3: LINK
    compile_link(CO);
    
}


int is_CF(char *str, CompilerFlags *CF)
{
    if (strcmp(str, "-ast") == 0) { CF->print_ast = 1; return 1; }
    if (strcmp(str, "-tok") == 0) { CF->print_tokens = 1; return 1; }
    if (strcmp(str, "-S") == 0) { CF->only_assemble = 1; return 1; }
    if (strcmp(str, "-debug") == 0) { CF->debug = 1; return 1; }
    if (strcmp(str, "-nl") == 0)  { CF->no_link = 1; return 1; }
    if (strcmp(str, "-time") == 0) { CF->time = 1; return 1; }
    
    return 0;
}

int is_CT(char *str, CompilerTerminators *CT)
{
    if (strcmp(str, "--help") == 0) { CT->help = 1; return 1; }

    return 0;
}

int is_CA(char *str, CompilerArgs *CA)
{
    static int argCount = 0; 

    if (str[0] == '-') { return 0; }
    if (argCount >= 2){
        fprintf(stderr, "Only two input arguments are allowed but got three. Run 'bjornc --help' for usage instructions.");
        exit(1);
    }
    
    // First argument specifies the input
    if (argCount == 0){
        CA->scriptPath = str;
        argCount++;
    }
    // Second argument specifies the output 
    else if (argCount == 1){
        CA->outputPath = str;
        argCount++;
    }

    return 1;
}

CompilerOptions *parse_args(int argc, char *argv[])
{
    // Create a compiler options object
    CompilerOptions *CO = malloc(sizeof(CompilerOptions));
    CO->CT = malloc(sizeof(CompilerTerminators));
    CO->CT->help = 0;
    CO->CA = malloc(sizeof(CompilerArgs));
    CO->CA->scriptPath = NULL;
    CO->CA->outputPath = NULL;

    CO->CF = malloc(sizeof(CompilerFlags));
    CO->CF->print_ast = 0;
    // Loop through the arguments 
    for (int i = 1; i < argc; i++)
    {
        if (is_CF(argv[i], CO->CF)) { continue; }
        if (is_CT(argv[i], CO->CT)) { continue; }
        if (is_CA(argv[i], CO->CA)) { continue; }

        fprintf(stderr, "Unexpected argument '%s'. Run 'bjornc --help' for instructions on usage.\n", argv[i]);
        exit(1);
    }
    
    return CO;
}

void run_terminators(CompilerTerminators *CT)
{
    if (CT->help)
    {
        printf("Below is an example of the correct Bjorn compiler usage. \n");
        printf("Square brackets '[]' indicate optional argument and '<>' indicate possibility of multiple arguments of the same type\n");
        printf("Terminators ---> Arguments that don't even start the compiling process. Used for queries about usage. Preffix: '--' \n");
        printf("Flags --> Arguments that modify the compilation process and/or show information about it. Preffix: '-' \n");
        printf("Correct usage is: \n 1. bjornc --terminator \n 2. bjornc <[-flags]> scriptPath.bjo outputPath\n");

        exit(0);
    }

}

void free_CO(CompilerOptions *CO)
{
    free(CO->CF);
    free(CO->CT);
    free(CO->CA);
    free(CO);
}

int main(int argc, char *argv[])
{
    
    // Ensure at least one argument is provided 
    if (argc < 2){
        fprintf(stderr, "No arguments were given to the compiler. Run 'bjornc --help' for instructions on usage.\n");
        exit(1);
    }
    
    //Parse arguments 
    CompilerOptions *CO = parse_args(argc, argv);
    
    //Check for terminators 
    run_terminators(CO->CT);
    
    // Compile the code
    compile(CO);


    //Free Compiler Options
    free_CO(CO);

    return 0;
}

 
