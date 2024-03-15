#include "base\baseMemory.h"

#ifndef baseArenaReserveImpl
#error baseArenaReserveImpl is not defined.
#endif
#ifndef baseArenaCommitImpl
#error baseArenaCommitImpl is not defined.
#endif
#ifndef baseArenaDecommitImpl
#error baseArenaDecommitImpl is not defined.
#endif
#ifndef baseArenaFreeImpl
#error baseArenaFreeImpl is not defined.
#endif

BaseArena *baseArenaAlloc(u64 size)
{
    u64 sizeToReserve = sizeof(BaseArena) + size;
    void *block = baseArenaReserveImpl(sizeToReserve);

    u64 initialCommitSize = sizeof(BaseArena) + BASE_ARENA_INITIAL_COMMIT;
    baseArenaCommitImpl(block, initialCommitSize);

    BaseArena *arena = (BaseArena*)block;
    arena->pos = sizeof(BaseArena);
    arena->size = sizeToReserve;
    arena->commitPos = initialCommitSize;
    arena->align = 8;
    
    return arena;
}
BaseArena *baseArenaAllocDefault(void)
{
    return baseArenaAlloc(BASE_ARENA_DEFAULT_SIZE);
}
void baseArenaFree(BaseArena *arena)
{
    baseArenaFreeImpl(arena, arena->size);
}

void *baseArenaPushNoZero(BaseArena *arena, u64 size)
{
    void *returnPtr = null;
    if (arena->pos + size <= arena->size)
    {
        u8 *arenaBase = (u8*) arena;
        u8 *aligned = (arenaBase + arena->pos) + (arena->align - 1);
        aligned -= ((u64)aligned) % arena->align;

        u64 alignAmount = (u64)(aligned - (arenaBase + arena->pos));
        returnPtr = aligned;

        arena->pos += alignAmount + size;
        if (arena->pos <= arena->size && (arena->commitPos < arena->pos))
        {
            u64 sizeToCommit = arena->pos - arena->commitPos;
            sizeToCommit += BASE_ARENA_DEFAULT_COMMIT_GRANULARITY - 1;
            sizeToCommit -= sizeToCommit % BASE_ARENA_DEFAULT_COMMIT_GRANULARITY;

            baseArenaCommitImpl(arenaBase + arena->commitPos, sizeToCommit);
            arena->commitPos += sizeToCommit;
        }
    }
    else
    {
        //todo: probs want a better error printed and stuff
        return null;
    }

    return returnPtr;
}
void *baseArenaPush(BaseArena *arena, u64 size)
{
    void* ptr = baseArenaPushNoZero(arena, size);

    //todo: zero out
    BASE_MEMZERO(ptr, size);
    return ptr;
}
void baseArenaPopTo(BaseArena *arena, u64 popTo)
{
    u64 minPos = sizeof(BaseArena);
    arena->pos = BASE_MAX(minPos, popTo);

    u64 posAlignedToCommitChunks = arena->pos + BASE_ARENA_DEFAULT_COMMIT_GRANULARITY - 1;
    posAlignedToCommitChunks -= posAlignedToCommitChunks % BASE_ARENA_DEFAULT_COMMIT_GRANULARITY;

    if((posAlignedToCommitChunks + BASE_ARENA_DECOMMIT_THRESHOLD) <= arena->commitPos)
    {
        u8 *base = (u8 *)arena;
        u64 sizeToDecommit = arena->commitPos - posAlignedToCommitChunks;
        baseArenaDecommitImpl(arena + posAlignedToCommitChunks, sizeToDecommit);

        arena->commitPos -= sizeToDecommit;
    }
}

BaseArenaTemp baseArenaTempBegin(BaseArena *arena)
{
    BaseArenaTemp t = {0};
    t.arena = arena;
    t.checkpointPos = arena->pos;

    return t;
}
void baseArenaTempEnd(BaseArenaTemp temp)
{
    baseArenaPopTo(temp.arena, temp.checkpointPos);
}
