#include <stdlib.h>



// 16 Mib
// Around 5 tokens per asm line -> enough to tokenize 52k LOC
#define ARENA_INIT_SIZE 16 * 1024 * 1024

typedef struct Arena Arena;

typedef enum PoolType
{
    POOL_TOK,
    POOL_AST,
    POOL_SYMBOL
}PoolType;


typedef struct Arena
{
    Arena *next;
    size_t pos;
    size_t left;
    size_t total;
    PoolType ptype;
}Arena;

typedef struct MainArena
{
    Arena *arena_tok;
    Arena *arena_ast;
    Arena *arena_symbol;
    int arena_count;
}MainArena;


extern MainArena *marena;

void init_arenas();
void reset_arenas();
void *arena_alloc(size_t sz, PoolType type);