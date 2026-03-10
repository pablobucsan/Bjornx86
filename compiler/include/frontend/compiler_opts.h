#ifndef ABC_COMPILER_OPTS_H
#define ABC_COMPILER_OPTS_H

typedef enum FileType
{
    FILE_UNKNOWN,
    FILE_BJORN,
    FILE_ASM,
    FILE_CUB
}FileType;

typedef struct CompilerFlags
{
    int help;                    // -h                  (prints help)

    int self;                    // -self               (uses bjornas2 instead of bjornas)
    int compile_and_assemble;    // -c                  (produces .cub)
    int compile;                 // -s                  (produces .asm)
    int specifies_opath;         // -o filepath         (output at filepath )
    int save_temps;              // -save-temps         (writes temp files to disk)
}CompilerFlags;


typedef struct InputFile
{
    char *path;
    char *name;
    char *extension;
    FileType type;
}InputFile;

typedef struct CompilerOptions
{
    CompilerFlags *flags;
    InputFile **inputfiles;
    char *outputpath;
    int ifiles_count;
}CompilerOptions;


// Global compiler opts, one per session
extern CompilerOptions *compiler_opts;


void parse_args(int argc, char **argv);
void printCompilerOptions();
char *getFileName(char *path);

#endif //ABC_COMPILER_OPTS_H