

#include <stdio.h>
#include "../../include/misc/arena.h"

MainArena *marena = NULL;


Arena *create_arena(size_t total_sz, PoolType ptype)
{
    Arena *arena = malloc(sizeof(Arena) + total_sz);
    arena->next = NULL;
    arena->pos = 0;
    arena->left = total_sz;
    arena->total = total_sz;

    return arena;
}

void restart(Arena *arena)
{
    if (arena == NULL){
        printf("Can't restart a null arena\n");
        exit(1);
    }

    arena->pos = 0;
    arena->left = arena->total;

}

void init_arenas()
{
    marena = malloc(sizeof(MainArena));
    marena->arena_count = 3;
    marena->arena_tok = create_arena(ARENA_INIT_SIZE, POOL_TOK);
    marena->arena_ast = create_arena(ARENA_INIT_SIZE, POOL_AST);
    marena->arena_symbol = create_arena(ARENA_INIT_SIZE, POOL_SYMBOL);

}


void reset_arenas()
{
    if (marena == NULL){
        return;
    }

    // Reset tok
    Arena *curr = marena->arena_tok;
    while (curr != NULL){
        restart(curr);
        curr = curr->next;
    }

    // Reset ast
    curr = marena->arena_ast;
    while (curr != NULL){
        restart(curr);
        curr = curr->next;
    }

    // Reset symbol
    curr = marena->arena_symbol;
    while (curr != NULL){
        restart(curr);
        curr = curr->next;
    }
}

size_t arena_align_to(size_t n, size_t multiple)
{
    size_t result = n + (multiple - 1) - ((multiple + n - 1) % multiple);
    return result;

}



void *arena_alloc(size_t sz, PoolType type)
{
    if (marena == NULL){
        printf("Main arena is NULL, cant allocate\n");
        exit(1);
    }

    Arena *head_arena = NULL;

    if (type == POOL_TOK){
        head_arena = marena->arena_tok;
    }
    else if (type == POOL_AST){
        head_arena = marena->arena_ast;
    }
    else if (type == POOL_SYMBOL){
        head_arena = marena->arena_symbol;
    }
    else{
        printf("Unknown pool type to get the head_arena from: %hhu\n", type);
        exit(1);
    }


    if (head_arena == NULL){
        printf("Head arena to allocate from is NULL, type: %hhu\n", type);
        exit(1);
    }

    size_t size = arena_align_to(sz, 8);
    // printf("Trying to arena allocate: %lu\n", size);

    Arena *prev_arena = NULL;
    Arena *curr_arena = head_arena;
    while (curr_arena != NULL && curr_arena->left < size){
        prev_arena = curr_arena;
        curr_arena = curr_arena->next;
    }

    //** 2 cases, both imply prev_arena != NULL **/
    //** 1. curr_arena = head_arena => curr_arena == NULL, but that means head_arena == NULL and we already check for that before **/
    //** 2. We looped N >= 1 times and curr_arena == NULL || curr_arena->left >= size, but that means prev != NULL **/

    Arena *target_arena = NULL;
    //** Not enough space in the current linked arenas chain, create a new one **/
    if (curr_arena == NULL){

        //** Just in case though **/
        if (prev_arena == NULL){
            printf("Previous arena is NULL, ive got no idea\n");
            exit(1);
        }
        //** Get a multiple of ARENA_INIT_SIZE bigger than or equal to size **/
        size_t new_size = prev_arena->total * 2;
        Arena *new = create_arena(new_size, type);
        //** Chain it **/
        prev_arena->next = new;

        target_arena = new;

    }
    else{
        target_arena = curr_arena;
    }

    if (target_arena == NULL){
        printf("I dont know how we got here but just for sanity check\n");
        exit(1);
    }


    void *p = target_arena + sizeof(Arena) + target_arena->pos;

    target_arena->pos += size;
    target_arena->left -= size;

    // printf("Returning p at : 0x%x\n", p);

    return p;

}