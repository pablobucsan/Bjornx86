#include "../include/tokenizer.h"
#include "../include/ast.h"
#include "../include/analyzer.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../include/builder.h"

void genExit(FILE *file);


typedef struct CompilerFlags
{
    int print_ast;
    int print_tokens;
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





void genData(FILE *file, ASTNode *program)
{
    fprintf(file, "section .data\n");
    buildData(file, program, gb_symbolTable);
    fprintf(file,"\n\n\n");
}

void genStart(FILE *file, ASTNode *program)
{
    fprintf(file, "section .text\n");
    fprintf(file, "\n; Extern functions \n");
    buildExtern(file, program, gb_functionTable);
    fprintf(file,"\n");
    fprintf(file, "global _start\n");
    fprintf(file, "_start:\n");
    fprintf(file, "    call _main\n");
    genExit(file);
    
    initWorkingContext();
    initMatcher();
    init_GPR();
    init_FPR();
    buildStart(file, program, gb_symbolTable, gb_functionTable);
}

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

void compile(CompilerOptions *CO)
{
    // Ensure input and output paths were given 
    if (CO->CA->scriptPath == NULL)
    {
        fprintf(stderr, "No input source file was specified. Run 'bjornc --help' for instructions on usage\n");
        exit(1);
    }

    if (CO->CA->outputPath == NULL)
    {
        fprintf(stderr, "No output destination file was specified. Run 'bjornc --help' for instructions on usage.\n");
        exit(1);
    }


    // TOKENIZE THE SOURCE CODE 
    char *src = read_file(CO->CA->scriptPath);
    Token **tokens = tokenize(src, CO->CA->scriptPath);
    int token_pos = 0; 
    
    if (CO->CF->print_tokens) { print_tokens(tokens); }
    
    // GENERATE AST REPRESENTATION
    init_usedFiles(5);
    ASTNode *program = parseProgram(tokens, &token_pos);
    
    if (CO->CF->print_ast) { print_ast(program,0); }
    

    
    // ANALYZE AND POPULATE SYMBOL, FUNCTION, OBJECT, ENUM TABLES
    init_gb_symbol_table(1);
    init_gb_function_table(1);
    init_gb_object_table(1);
    init_gb_enum_table(1); 
    

    analyze(program, gb_symbolTable, gb_functionTable, SCRIPT_USER); 
        
    printf("Done analyzing\n");    
    // BUILD AND GENERATE x86_64 INSTRUCTIONS
    char *file = CO->CA->outputPath;
    char *buffer = ".asm\0";

    char *totalPath = strcat(file, buffer);
    FILE *asm_file = fopen(totalPath, "w");

    // FIRST PASS FOR DATA     
    genData(asm_file, program);
        

    // SECOND PASS FOR _START
    genStart(asm_file, program);
    
    
    printf("Built successfully.\n");
}


int is_CF(char *str, CompilerFlags *CF)
{
    if (strcmp(str, "-ast") == 0) { CF->print_ast = 1; return 1; }
    if (strcmp(str, "-tok") == 0) { CF->print_tokens = 1; return 1; }

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
    if (argCount >= 2)
    {
        fprintf(stderr, "Only two input arguments are allowed but got three. Run 'bjornc --help' for usage instructions.", 
                CA->scriptPath, str);
        exit(1);
    }
    
    // First argument specifies the input
    if (argCount == 0)
    {
        CA->scriptPath = str;
        argCount++;
    }
    // Second argument specifies the input 
    else if (argCount == 1)
    {
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
        printf("Correct usage is: \n 1. bjornc --terminator \n 2. bjornc <[-flags]> outputPath scriptPath.bjo\n");

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
    if (argc < 2)
    {
        fprintf(stderr, "No arguments were given to the compiler. Run 'bjornc --help' for instructions on usage.\n");
        exit(1);
    }
    
    //Parse arguments 
    CompilerOptions *CO = parse_args(argc, argv);
    
    //Check for terminators 
    run_terminators(CO->CT);
    
    //Compile the code 
    compile(CO);
    
    //Free Compiler Options
    free_CO(CO);

    return 0;
}

 
