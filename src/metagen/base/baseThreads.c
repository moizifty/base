#include "baseCore.h"
#include "baseLog.h"
#include "baseMemory.h"
#include "baseThreads.h"

threadlocal BaseThreadCtx *tlThreadCtx;

BaseThreadCtx baseThreadsCreateCtx(void)
{
    BaseThreadCtx ctx = {0};
    for(int i = 0; i < BASE_THREADS_NUM_ARENAS; i++)
    {
        ctx.scratchArenas[i] = baseArenaAlloc(BASE_THREADS_DEFAULT_ARENA_ALLOC_SIZE);
    }
    
    ctx.threadLogArena = baseArenaAlloc(BASE_THREADS_DEFAULT_ARENA_ALLOC_SIZE);
    ctx.threadLog = logCreate(ctx.threadLogArena);
    
    return ctx;
}
BaseThreadCtx *baseThreadsGetCtx(void)
{
    return tlThreadCtx;
}
void baseThreadsSetCtx(BaseThreadCtx *ctx)
{
    tlThreadCtx = ctx;
}

BaseArenaTemp baseTempBegin(BaseArena **conflictsToCheck, u64 count)
{
    BaseArenaTemp temp = {0};
    BaseThreadCtx *ctx = baseThreadsGetCtx();

    for(u64 i = 0; i < BASE_THREADS_NUM_ARENAS; i++)
    {
        bool isConflicting = false;
        for(u64 c = 0; c < count; c++)
        {
            BaseArena *conflict = conflictsToCheck[c];
            if(conflict == ctx->scratchArenas[i])
            {
                isConflicting = true;
                break;
            }
        }

        if(!isConflicting)
        {
            temp.arena = ctx->scratchArenas[i];
            temp.checkpointPos = temp.arena->pos;
            break;
        }
    }
    
    return temp;
}
void baseTempEnd(BaseArenaTemp temp)
{
    baseArenaTempEnd(temp);
}

void baseThreadsSetName(str8 name)
{
    BaseThreadCtx *threadCtx = baseThreadsGetCtx();
    u64 nameLen = BASE_CLAMP(name.len, 0, BASE_ARRAY_SIZE(threadCtx->threadNameBuffer));
    BASE_MEMCPY(threadCtx->threadNameBuffer, name.data, nameLen);
    threadCtx->threadNameLen = nameLen;
}
str8 baseThreadsGetName(void)
{
    BaseThreadCtx *threadCtx = baseThreadsGetCtx();
    return (str8){.data = threadCtx->threadNameBuffer, .len = threadCtx->threadNameLen};
}

void baseThreadsSetLog(Log *log)
{
    BaseThreadCtx *threadCtx = baseThreadsGetCtx();
    threadCtx->threadLog = log;
}
Log *baseThreadsGetLog(void)
{
    BaseThreadCtx *threadCtx = baseThreadsGetCtx();

    return threadCtx->threadLog;
}