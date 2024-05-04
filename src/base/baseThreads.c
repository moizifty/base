#include "baseMemory.h"
#include "baseThreads.h"

threadlocal BaseThreadCtx *tl_ctx;

BaseThreadCtx baseThreadsCreateCtx(void)
{
    BaseThreadCtx ctx = {0};
    for(int i = 0; i < BASE_THREADS_NUM_ARENAS; i++)
    {
        ctx.scratchArenas[i] = baseArenaAlloc(BASE_THREADS_DEFAULT_ARENA_ALLOC_SIZE);
    }
    
    return ctx;
}
BaseThreadCtx *baseThreadsGetCtx(void)
{
    return tl_ctx;
}
void baseThreadsSetCtx(BaseThreadCtx *ctx)
{
    tl_ctx = ctx;
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
