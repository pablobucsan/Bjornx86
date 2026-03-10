

#define _GNU_SOURCE
#include "../../include/frontend/pipeline.h"
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <stdarg.h>
#include <dirent.h>
#include <errno.h>
#include <limits.h>
#include <sys/types.h>
#include "../../include/backend/builder.h"
#include "../../include/misc/arena.h"
#include "../../include/misc/errors.h"

OutputStash *ostash = NULL;


int mkdir_p(const char *path)
{
    char tmp[PATH_MAX];
    strncpy(tmp, path, sizeof(tmp));
    tmp[sizeof(tmp) - 1] = '\0';

    for (char *p = tmp + 1; *p; p++){
        if (*p == '/'){
            *p = '\0';
            if (mkdir(tmp, 0700) < 0 && errno != EEXIST){
                return -1;
            }
            *p = '/';
        }
    }
    if (mkdir(tmp, 0700) < 0 && errno != EEXIST){
        return -1;
    }
    return 0;
}

void remove_dir_recursive(const char *path)
{
    DIR *dir = opendir(path);
    if (dir == NULL) return;

    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL){
        // Skip . and ..
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) continue;

        // Build full path
        char full_path[PATH_MAX];
        snprintf(full_path, sizeof(full_path), "%s/%s", path, entry->d_name);

        if (entry->d_type == DT_DIR){
            remove_dir_recursive(full_path);
        } else {
            remove(full_path);
        }
    }

    closedir(dir);
    rmdir(path);
}

void delete_unwanted_files()
{
    /** Decide to whether delete or keep the files */
    if (compiler_opts->flags->save_temps) {
        return;
    }

    char *removed_dirs[ostash->count];
    int removed_count = 0;

    for (int i = 0; i < ostash->count; i++){
        if (!ostash->bfiles[i]->is_temp){
            continue;
        }
            
        char *dir = ostash->bfiles[i]->directory;
        int already_removed = 0;
        
        for (int j = 0; j < removed_count; j++){
            if (strcmp(removed_dirs[j], dir) == 0){ already_removed = 1; break; }
        }
        if (!already_removed){
            remove_dir_recursive(dir);
            removed_dirs[removed_count++] = dir;
        }
    }
}

void start_tracking(char *path)
{
    tracker.current_line = 1;
    tracker.current_src_file = strdup(path);
}

void stop_tracking()
{
    tracker.current_line = 1;
    tracker.current_src_file = NULL;
}

void append_to_stream(char **stream, size_t *size, size_t *pos, const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);

    va_list args_copy;
    va_copy(args_copy, args);
    int needed = vsnprintf(NULL, 0, fmt, args_copy);
    va_end(args_copy);

    if (needed < 0) {
        perror("vsnprintf size calc failed");
        exit(1);
    }

    // Add space for null terminator + some extra breathing room
    size_t required = *pos + needed + 1 + 16;  // +16 safety margin
    if (required > *size) {
        // Grow to at least required, or double if already large
        size_t new_size = *size ? *size * 2 : 1024;
        if (new_size < required) new_size = required;

        char *new_stream = realloc(*stream, new_size);
        if (new_stream == NULL) {
            perror("realloc failed in append_to_stream");
            exit(1);
        }
        *stream = new_stream;
        *size = new_size;
    }

    // Now write
    int written = vsnprintf(*stream + *pos, *size - *pos, fmt, args);
    if (written < 0 || written > needed) {
        perror("vsnprintf write failed");
        exit(1);
    }

    *pos += written;

    va_end(args);
}

void initOutputStash()
{
    ostash = malloc(sizeof(OutputStash));
    ostash->capacity = INIT_BOXEDFILE_CAP;
    ostash->count = 0;
    ostash->bfiles = malloc(sizeof(BoxedFile *) * ostash->capacity);
}

void push_bfile(BoxedFile *bfile)
{
    if (ostash->count + 1 >= ostash->capacity){
        ostash->capacity *= 2;
        ostash->bfiles = realloc(ostash->bfiles, sizeof(BoxedFile *) * ostash->capacity);
    }
    ostash->bfiles[ostash->count++] = bfile;
}

BoxedFile *makeBoxedFile(char *path_to_result, char *directory, FileType type, int is_temp)
{
    BoxedFile *bfile = malloc(sizeof(BoxedFile));
    bfile->path_to_result = strdup(path_to_result);
    bfile->directory = strdup(directory);
    bfile->type = type;
    bfile->name = getFileName(path_to_result);
    bfile->is_temp = is_temp;

    return bfile;
}

void setup_session()
{
    init_arenas();
    /** For stashing boxed files */
    initOutputStash();

    /** For errors */
    __initErrorManager();

}

void end_session()
{

    /** Delete unwanted files and directories */
    delete_unwanted_files();
}

void setup_compilation(char *filename)
{
    /** For tokenizing */
    initTokenizerContext();

    /** For parsing */
    initASTContext();

    /** For analyzing */
    initAnalysisContext();
    initGlobalTables();
    __initGlobalStringTable(filename);

    /** For building */
    initMatcher();
    init_GPR();
    init_FPR();
}

void end_compilation()
{
    reset_arenas();
}

void pipeit(InputFile **inputfiles, int count)
{
    /** Compile into .asm every .bjo file */
    for (int i = 0; i < count; i++){
        /** Decide what to do based on the file type */
        switch(inputfiles[i]->type)
        {
            case FILE_BJORN:{
                /** Produce .asm and put them in the stash */
                setup_compilation(inputfiles[i]->name);
                bjornc(inputfiles[i]);
                break;
            }
            default:{
                // Ignore
                break;
            }
        }
    }

    if (compiler_opts->flags->compile){
        /** We are done, return */
        return;
    }

    /** Proceed to produce .cub files */
    bjornas();

    if (compiler_opts->flags->compile_and_assemble){
        /** We are done, return */
        return;
    }

    /** Proceed to produce final .elf file */
    bjornlk();
}






void bjornc(InputFile *ifile)
{
    /** Stage 1: Tokenize */
    start_tracking(ifile->path);

    char *src = read_file(ifile->path);
    Token **tokens = tokenize(src, ifile->path);

    // printf("Done tokenizing\n");
    /** Stage 2: Parse the tokens */
    int token_pos = 0;
    ASTNode *program = parseProgram(tokens, &token_pos);
    // printf("---  DONE PARSING ---- \n");

    /** Stage 3: Analyze the AST tree */
    analyze(program, gb_symbolTable, gb_functionTable);
    resetContext();
    errorManagerCheckInBeforeBuild();
    // printf("---- DONE ANALYZING -----\n");

    /** Stage 4: Build and emit the instructions */
    /** Use a stream instead of the file in case we dont wanna write to disk */
    char *asm_buffer = NULL;
    size_t asm_size = 0;
    FILE *asm_file = open_memstream(&asm_buffer, &asm_size);

    /** Can skip setup initialization and extern, we will link with a .cub file for that*/
    /** section .data before collecting strings so the labels belong to the .data section!! */
    fprintf(asm_file, "section .data\n");
    /** Collect strings */
    /** Globlal data */
    collectStrings(asm_file, program);
    buildData(asm_file, program, gb_symbolTable, gb_functionTable);
    /** Instructions */
    fprintf(asm_file, "section .text\n");
    // printf("About to build start\n");
    buildStart(asm_file, program, gb_symbolTable, gb_functionTable);
    fclose(asm_file);   // Must close before accessing its attributes!!!
    
    // printf("---- DONE BUILDING ---- \n");
    /** Stop tracking so we dont populate new errors with stale src files */
    stop_tracking();

    /** Decide whether to write to temps vs write to a folder the user wants to keep */
    int is_temp = 0;
    char *directory;
    if (compiler_opts->flags->save_temps){
        directory = strdup("./saved-temps/bjornc");
        int result = mkdir_p(directory);
        if (result < 0){
            /** Failed to make saved dir */
            Error *e = invoke(FAILED_TO_CREATE_DIR);
            appendMessageToError(e, fetchErrorMsgBasedOnErrorID(e->error_id), directory);
            addError(e);
        }

    }
    else{  /** We dont want to keep them, write to temp */
        is_temp = 1;
        char template[] = "/tmp/bjornc-XXXXXX";
        directory = mkdtemp(template);
        if (directory == NULL){
            /** Failed to make temporary dir */
            Error *e = invoke(FAILED_TO_CREATE_DIR); 
            appendMessageToError(e, fetchErrorMsgBasedOnErrorID(e->error_id), directory);
            addError(e);
        }
    }

    /** Write it to temps */
    char *filepath = malloc(256);
    memset(filepath, 0, 256);
    sprintf(filepath, "%s/%s.asm", directory, ifile->name);
    // printf("Done here, filename = %s\n", filepath);
    

    FILE *f = fopen(filepath, "wb");
    fwrite(asm_buffer, 1, asm_size, f);
    fclose(f);

    
    /** Push the result boxed file to the output stash */
    BoxedFile *bfile = makeBoxedFile(filepath, directory, FILE_ASM, is_temp);
    push_bfile(bfile);
    
    // printf("Made a bfile with path: '%s'\n", bfile->path_to_result);
    // printf("Finished bjornc\n");


}


void bjornas()
{

    /** Decide whether to write to temps vs write to a folder the user wants to keep */
    char *directory;
    int is_temp = 0;
    if (compiler_opts->flags->save_temps){
        directory = strdup("./saved-temps/bjornas");
        int result = mkdir_p(directory);
        if (result < 0){
            /** Failed to make saved dir */
            Error *e = invoke(FAILED_TO_CREATE_DIR);
            appendMessageToError(e, fetchErrorMsgBasedOnErrorID(e->error_id), directory);
            addError(e);
        }
    }
    else{
        is_temp = 1;
        char template[] = "/tmp/bjornas-XXXXXX";
        directory = strdup(mkdtemp(template));
        if (directory == NULL){
            /** Failed to make temporary dir */
            Error *e = invoke(FAILED_TO_CREATE_DIR); 
            appendMessageToError(e, fetchErrorMsgBasedOnErrorID(e->error_id), directory);
            addError(e);
        }
    }

    /** Create the bjornas command and locate every .cub file */
    size_t pos = 0;
    char *stream = malloc(1024);
    size_t stream_size = 1024;

    char *assembler = NULL;
    if (compiler_opts->flags->self){
        assembler = "bjornas2.elf";
    }
    else{
        assembler = "bjornas";
    }

    append_to_stream(&stream, &stream_size, &pos, "%s ", assembler);

    for (int i = 0; i < ostash->count; i++){
        
        if (ostash->bfiles[i]->type != FILE_ASM){
            continue;
        }
        // printf("About to append to stream\n");
        append_to_stream(&stream, &stream_size, &pos, "%s ", ostash->bfiles[i]->path_to_result);

        
        /** Create the boxed cub file and push it */
        size_t filepath_len = strlen(directory) + strlen(ostash->bfiles[i]->name) + strlen(".cub") + 1;
        char *filepath = malloc(filepath_len);
        memset(filepath, 0, filepath_len);
        // printf("Before: Assembling, dir = %s\n", directory);
        sprintf(filepath, "%s/%s.cub", directory, ostash->bfiles[i]->name);
        // printf("After: Assembling, dir = %s\n", directory);

        BoxedFile *bfile = makeBoxedFile(filepath, directory, FILE_CUB, is_temp);
        push_bfile(bfile);
        // printf("Finished iteration\n");
    }
    
    /** For bjornas to put the .cub files in the directory */
    append_to_stream(&stream, &stream_size, &pos, " -cub %s", directory);
    append_to_stream(&stream, &stream_size, &pos, "\0");

    // printf("Command would be: '%s'\n", stream);


    int status = system(stream);
    // printf("Raw status: %d\n", status);

    if (WIFEXITED(status)) {
        int exit_code = WEXITSTATUS(status);
        // printf("Exit code: %d\n", exit_code);
        
        if (exit_code != 0) {
            printf("bjornas failed!\n");
            
            // Try running it manually to see errors:
            printf("Try running manually:\n%s\n", stream);
            exit(1);
        }
    }


    // printf("Done assembling\n");
}


void bjornlk()
{

    /** Create the bjornlk */
    size_t pos = 0;
    char *stream = malloc(1024);
    size_t stream_size = 1024;


    char *linker = NULL;
    if (compiler_opts->flags->self){
        linker = "bjornlk2.elf";
    }
    else{
        linker = "bjornlk";
    }

    append_to_stream(&stream, &stream_size, &pos, "%s ", linker);

    /** Create the bjornlk command */

    for (int i = 0; i < ostash->count; i++){

        if (ostash->bfiles[i]->type != FILE_CUB){
            continue;
        }

        append_to_stream(&stream, &stream_size, &pos, "%s ", ostash->bfiles[i]->path_to_result);
     
    }
    
    /** Append std cubs*/
    char *sys_lib_dir = get_system_lib_dir();
    char *stdcubs_path = path_join(sys_lib_dir,"cubs");

    append_to_stream(&stream, &stream_size, &pos, "%s/*.cub ", stdcubs_path);

    char *outpath = "a.elf";

    if (compiler_opts->flags->specifies_opath){
        outpath = compiler_opts->outputpath;
        sprintf(&(stream[pos]), "-exe %s", outpath);
    }


    // printf("Command would be: '%s'\n", stream);

    int status = system(stream);
    if (status != 0){
        printf("bjornlk failed\n");

        // Try running it manually to see errors:
        printf("Try running manually:\n%s\n", stream);
        exit(1);
    }

    // printf("Done linking\n");
}