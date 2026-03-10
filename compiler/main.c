#include "include/frontend/tokenizer.h"
#include "include/frontend/ast.h"
#include "include/backend/analyzer.h"
#include "include/backend/builder.h"
#include "include/frontend/compiler_opts.h"
#include "include/frontend/pipeline.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>  // for getcwd
#include <limits.h>  // for PATH_MAX
#include <time.h>


void printHelp()
{
    const char *help = "Usage:\n\n"
"bjornc [flags] <input.bjo> [<input2.bjo>...]\n\n"
"Flags:\n"
"[no flags]         Produces just the final executable <a.elf>\n"
"-h                 Prints this message, does not proceed to compile\n"
"-save-temps        Saves every intermediate file (.asm and .cub files) into saved-temps/bjornc and saved-temps/bjornas\n"
"-s                 Produces and saves the .asm generated file(s), does not assemble nor link\n"
"-c                 Produces and saves the .asm and .cub generated file(s), does not link\n"
"-o <exename>       Names the final executable file to <exename>.elf\n";

    printf("%s", help);
}


int main(int argc, char *argv[])
{
    
    // Ensure at least one argument is provided 
    if (argc < 2){
        fprintf(stderr, "No arguments were given to the compiler. Run 'bjornc -h' for instructions on usage.\n");
        exit(1);
    }
    
    //Parse arguments 
    parse_args(argc, argv);

    // Print the help if that is the case
    if (compiler_opts->flags->help){
        printHelp();
        return 0;
    }


    
    // printCompilerOptions();
    setup_session();

    pipeit(compiler_opts->inputfiles, compiler_opts->ifiles_count);

    // End of session
    end_session();

    // Compile the code
    // compile(CO);

    return 0;
}

 
