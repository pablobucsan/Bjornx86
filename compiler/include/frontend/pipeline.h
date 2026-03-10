
#ifndef ABC_PIPELINE_H
#define ABC_PIPELINE_H

#include "compiler_opts.h"
#include "ast.h"
#include <stdio.h>


#define INIT_BOXEDFILE_CAP 10

typedef struct BoxedFile
{   
    char *path_to_result;
    char *directory;
    char *name;
    int is_temp;
    FileType type;
}BoxedFile;

typedef struct OutputStash
{
    BoxedFile **bfiles;
    int capacity;
    int count;
}OutputStash;



extern OutputStash *ostash;

void setup_session();
void end_session();
void pipeit(InputFile **inputfiles, int count);
void bjornc(InputFile *ifile);
void bjornas();
void bjornlk();

#endif //ABC_PIPELINE_H