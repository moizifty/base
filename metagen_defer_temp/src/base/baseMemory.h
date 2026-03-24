#ifndef BASE_MEMORY_H
#define BASE_MEMORY_H

#include "baseCoreTypes.h"

#define BASE_ARENA_INITIAL_COMMIT (BASE_KILOBYTES(4))
#define BASE_ARENA_DEFAULT_SIZE (BASE_GIGABYTES(1))
#define BASE_ARENA_DEFAULT_COMMIT_GRANULARITY (BASE_KILOBYTES(4))
#define BASE_ARENA_DEFAULT_DECOMMIT_GRANULARITY (BASE_KILOBYTES(4))
#define BASE_ARENA_DECOMMIT_THRESHOLD (BASE_MEGABYTES(64))

#define arenaPushType(arena, TYPE)  ((TYPE*)arenaPush(arena, sizeof(TYPE)))
#define arenaPushArray(arena, TYPE, COUNT)  ((TYPE*)arenaPush(arena, sizeof(TYPE) * (COUNT)))

// the data ptr for the arena is the arena pointer itself, returned
typedef struct Arena
{
    u64 pos;
    u64 commitPos;
    u64 align;
    u64 size;
}Arena;

typedef struct ArenaTemp
{
    Arena *arena;
    u64 checkpointPos;
}ArenaTemp;

Arena *arenaAlloc(u64 size);
Arena *arenaAllocDefault(void);
void arenaFree(Arena *arena);

void *arenaPushNoZero(Arena *arena, u64 size);
void *arenaPush(Arena *arena, u64 size);
void arenaPopTo(Arena *arena, u64 popTo);

ArenaTemp arenaTempBegin(Arena *arena);
void arenaTempEnd(ArenaTemp temp);

#endif