
#include "../../include/frontend/compiler_opts.h"
#include "../../include/misc/errors.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

CompilerOptions *compiler_opts = NULL;

char *trimright(char *stream, int position)
{
    int len = strlen(stream);
    if (position >= len || position < 0){
        return NULL;
    }

    char *remainder = malloc(len - position + 1);
    memset(remainder, 0, len - position + 1);
    memcpy(remainder, &(stream[position]), len - position);

    return remainder;
}

char *trimleft(char *stream, int position)
{
    int len = strlen(stream);
    if (position >= len || position < 0){
        return NULL;
    }

    char *remainder = malloc(position + 1);
    memset(remainder, 0, position + 1);
    memcpy(remainder, &(stream[0]), position);

    return remainder;
}

int findCharRightToLeft(char *stream, char c)
{
    int len = strlen(stream);
    for (int i = len - 1; i >= 0; i--){
        if (stream[i] == c){
            return i;
        }
    }
    return -1;
}

FileType getFileType(char *ext)
{
    if (ext == NULL){
        return FILE_UNKNOWN;
    }
    if (strcmp(ext,"bjo") == 0 || strcmp(ext,"bjorn") == 0){
        return FILE_BJORN;
    }
    if (strcmp(ext, "asm") == 0){
        return FILE_ASM;
    }
    if (strcmp(ext, "cub") == 0){
        return FILE_CUB;
    }

    return FILE_UNKNOWN;
}

char *getFileName(char *path)
{

    
    int slash_index = findCharRightToLeft(path, '/');
    int start_index = slash_index + 1;
    if (slash_index == -1){
        start_index = 0;
    }
    char *name_and_ext = trimright(path, start_index);

    int dot_index = findCharRightToLeft(name_and_ext, '.');
    if (dot_index == -1){
        return NULL;
    }

    char *name = trimleft(name_and_ext, dot_index);
    return name;
}

char *getExtension(char *path)
{
    int dot_index = findCharRightToLeft(path, '.');
    if (dot_index == -1){
        return NULL;
    }

    char *extension = trimright(path, dot_index + 1);
    return extension;
}


InputFile *makeInputFile(char *path)
{
    InputFile *infile = malloc(sizeof(InputFile));
    infile->path = strdup(path);
    infile->name = getFileName(path);
    infile->extension = getExtension(path);
    infile->type = getFileType(infile->extension);

    // if (infile->name == NULL){
    //     printf("Input file '%s' has no valid filename\n", path);
    //     exit(1);
    //     Error *e = invoke(INVALID_FILENAME);
    //     appendMessageToError(e, fetchErrorMsgBasedOnErrorID(e->error_id), path);
    //     appendMessageToError(e, "Could not deduce file extension from it.");
    //     addError(e);
    // }

    if (infile->extension == NULL){
        printf("Input file '%s' has no extension, unclear to know what to do with it.\n", path);
        exit(1);
    }

    if (strcmp(infile->extension, "bjo") != 0){
        printf("Input file: '%s' is not a recognized Bjorn source code file (its extension is not '.bjo')\n", path);
        exit(1);
    }

    return infile;
}

void parse_args(int argc, char **argv)
{
    compiler_opts = malloc(sizeof(CompilerOptions));
    memset(compiler_opts, 0, sizeof(CompilerOptions));

    compiler_opts->flags = malloc(sizeof(CompilerFlags));
    memset(compiler_opts->flags, 0, sizeof(CompilerFlags));
    compiler_opts->inputfiles = malloc(sizeof(InputFile *) * argc);  // Allocate at most argc filepaths
    compiler_opts->ifiles_count = 0;
    compiler_opts->outputpath = NULL;

    
    for (int i = 1; i < argc; i++){
        if (strcmp(argv[i], "-c") == 0){
            compiler_opts->flags->compile_and_assemble = 1;
            continue;
        }
        if (strcmp(argv[i], "-s") == 0){
            compiler_opts->flags->compile = 1;
            continue;
        }
        if (strcmp(argv[i], "-h") == 0){
            compiler_opts->flags->help = 1;
            continue;
        }
        if (strcmp(argv[i],"-self") == 0){
            compiler_opts->flags->self = 1;
            continue;
        }
        if (strcmp(argv[i], "-o") == 0){
            i++;
            if (i >= argc){
                Error *e = invoke(EXPECTED_OUTPUT_FILEPATH);
                appendMessageToError(e, fetchErrorMsgBasedOnErrorID(e->error_id));
                appendMessageToError(e, "Run 'bjornc -help' for instructions on usage.");
                addError(e);
                
            }
            compiler_opts->flags->specifies_opath = 1;
            compiler_opts->outputpath = strdup(argv[i]);
            continue;
        }
        if (strcmp(argv[i], "-save-temps") == 0){
            compiler_opts->flags->save_temps = 1;
            continue;
        }

        // If we get this far, assume is an inputpath
        compiler_opts->inputfiles[compiler_opts->ifiles_count] = makeInputFile(argv[i]);
        // Make sure its a valid extension
        if (compiler_opts->inputfiles[compiler_opts->ifiles_count]->type == FILE_UNKNOWN){
            Error *e = invoke(UNKNOWN_FILE_EXT);
            appendMessageToError(e, fetchErrorMsgBasedOnErrorID(e->error_id));
            addError(e);
        }
        compiler_opts->ifiles_count++;
    }
}

void printCompilerOptions()
{
    printf("-c? %i\n", compiler_opts->flags->compile_and_assemble);
    printf("-s? %i\n", compiler_opts->flags->compile);
    printf("-o? %i\n", compiler_opts->flags->specifies_opath);

    for (int i = 0; i < compiler_opts->ifiles_count; i++){
        printf("Input file: '%s', name : '%s', extension: '%s', type: %i.\n", compiler_opts->inputfiles[i]->path,
                compiler_opts->inputfiles[i]->name,
                compiler_opts->inputfiles[i]->extension,
                compiler_opts->inputfiles[i]->type );
    }

}