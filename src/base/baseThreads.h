#ifndef BASE_THREADS_H
#define BASE_THREADS_H

#include "baseCoreTypes.h"

#define BASE_THREADS_NUM_ARENAS 2
#define BASE_THREADS_DEFAULT_ARENA_ALLOC_SIZE BASE_GIGABYTES(4)

typedef struct BaseThreadCtx
{
    Arena *scratchArenas[BASE_THREADS_NUM_ARENAS];

    u8 threadNameBuffer[64];
    u64 threadNameLen;

    Arena *threadLogArena;
    struct Log *threadLog;
}BaseThreadCtx;

BaseThreadCtx baseThreadsCreateCtx(void);
BaseThreadCtx *baseThreadsGetCtx(void);
void baseThreadsSetCtx(BaseThreadCtx *ctx);

ArenaTemp baseTempBegin(Arena **conflictsToCheck, u64 count);
void baseTempEnd(ArenaTemp temp);

void baseThreadsSetName(str8 name);
str8 baseThreadsGetName(void);

void baseThreadsSetLog(struct Log *log);
struct Log *baseThreadsGetLog(void);
#endif