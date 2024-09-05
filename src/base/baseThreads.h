#ifndef BASE_THREADS_H
#define BASE_THREADS_H

#include "baseCoreTypes.h"

#define BASE_THREADS_NUM_ARENAS 2
#define BASE_THREADS_DEFAULT_ARENA_ALLOC_SIZE BASE_GIGABYTES(4)

typedef struct BaseThreadCtx
{
    BaseArena *scratchArenas[BASE_THREADS_NUM_ARENAS];

    u8 threadNameBuffer[64];
    u64 threadNameLen;
}BaseThreadCtx;

BaseThreadCtx baseThreadsCreateCtx(void);
BaseThreadCtx *baseThreadsGetCtx(void);
void baseThreadsSetCtx(BaseThreadCtx *ctx);

BaseArenaTemp baseTempBegin(BaseArena **conflictsToCheck, u64 count);
void baseTempEnd(BaseArenaTemp temp);

#endif