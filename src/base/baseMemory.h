#ifndef BASE_MEMORY_H
#define BASE_MEMORY_H

#include "baseCoreTypes.h"

#define BASE_ARENA_INITIAL_COMMIT (BASE_KILOBYTES(4))
#define BASE_ARENA_DEFAULT_SIZE (BASE_GIGABYTES(1))
#define BASE_ARENA_DEFAULT_COMMIT_GRANULARITY (BASE_KILOBYTES(4))
#define BASE_ARENA_DEFAULT_DECOMMIT_GRANULARITY (BASE_KILOBYTES(4))
#define BASE_ARENA_DECOMMIT_THRESHOLD (BASE_MEGABYTES(64))

// the data ptr for the arena is the arena pointer itself, returned
typedef struct BaseArena
{
    u64 pos;
    u64 commitPos;
    u64 align;
    u64 size;
}BaseArena;

typedef struct BaseArenaTemp
{
    BaseArena *arena;
    u64 checkpointPos;
}BaseArenaTemp;

BaseArena *baseArenaAlloc(u64 size);
BaseArena *baseArenaAllocDefault(void);
void baseArenaFree(BaseArena *arena);

void *baseArenaPushNoZero(BaseArena *arena, u64 size);
void *baseArenaPush(BaseArena *arena, u64 size);
void baseArenaPopTo(BaseArena *arena, u64 popTo);

BaseArenaTemp baseArenaTempBegin(BaseArena *arena);
void baseArenaTempEnd(BaseArenaTemp temp);

#endif