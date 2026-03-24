#include "base/baseMemory.h"

#ifndef arenaReserveImpl
#error arenaReserveImpl is not defined.
#endif
#ifndef arenaCommitImpl
#error arenaCommitImpl is not defined.
#endif
#ifndef arenaDecommitImpl
#error arenaDecommitImpl is not defined.
#endif
#ifndef arenaFreeImpl
#error arenaFreeImpl is not defined.
#endif

Arena *arenaAlloc(u64 size)
{
    u64 sizeToReserve = sizeof(Arena) + size;
    void *block = arenaReserveImpl(sizeToReserve);

    u64 initialCommitSize = sizeof(Arena) + BASE_ARENA_INITIAL_COMMIT;
    arenaCommitImpl(block, initialCommitSize);

    Arena *arena = (Arena*)block;
    arena->pos = sizeof(Arena);
    arena->size = sizeToReserve;
    arena->commitPos = initialCommitSize;
    arena->align = 8;
    
    return arena;
}
Arena *arenaAllocDefault(void)
{
    return arenaAlloc(BASE_ARENA_DEFAULT_SIZE);
}
void arenaFree(Arena *arena)
{
    arenaFreeImpl(arena, arena->size);
}

void *arenaPushNoZero(Arena *arena, u64 size)
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
            u64 sizeToCommit = arena->pos + arena->commitPos;
            sizeToCommit += BASE_ARENA_DEFAULT_COMMIT_GRANULARITY - 1;
            sizeToCommit -= sizeToCommit % BASE_ARENA_DEFAULT_COMMIT_GRANULARITY;

            
            arenaCommitImpl(arenaBase, sizeToCommit);
            arena->commitPos = sizeToCommit;
        }
    }
    else
    {
        //todo: probs want a better error printed and stuff
        return null;
    }

    return returnPtr;
}
void *arenaPush(Arena *arena, u64 size)
{
    void* ptr = arenaPushNoZero(arena, size);

    memset(ptr, 0, size);
    BASE_MEMZERO(ptr, size);
    return ptr;
}
void arenaPopTo(Arena *arena, u64 popTo)
{
    u64 minPos = sizeof(Arena);
    arena->pos = BASE_MAX(minPos, popTo);

    u64 posAlignedToCommitChunks = arena->pos + BASE_ARENA_DEFAULT_COMMIT_GRANULARITY - 1;
    posAlignedToCommitChunks -= posAlignedToCommitChunks % BASE_ARENA_DEFAULT_COMMIT_GRANULARITY;

    if((posAlignedToCommitChunks + BASE_ARENA_DECOMMIT_THRESHOLD) <= arena->commitPos)
    {
        u8 *base = (u8 *)arena;
        u64 sizeToDecommit = arena->commitPos - posAlignedToCommitChunks;
        arenaDecommitImpl(base + posAlignedToCommitChunks, sizeToDecommit);

        arena->commitPos -= sizeToDecommit;
    }
}

ArenaTemp arenaTempBegin(Arena *arena)
{
    ArenaTemp t = {0};
    t.arena = arena;
    t.checkpointPos = arena->pos;

    return t;
}
void arenaTempEnd(ArenaTemp temp)
{
    arenaPopTo(temp.arena, temp.checkpointPos);
}
