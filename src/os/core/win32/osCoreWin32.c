#include "osCoreWin32.h"

void* OSReserveMemory(u64 size)
{
    return VirtualAlloc(NULL, size, MEM_RESERVE, PAGE_NOACCESS);
}
void OSCommitMemory(void *ptr, u64 size)
{
    VirtualAlloc(ptr, size, MEM_COMMIT, PAGE_READWRITE);
}
void OSDecommitMemory(void *ptr, u64 size)
{
    VirtualFree(ptr, size, MEM_DECOMMIT);
}
void OSFreeMemory(void *ptr, u64 size)
{
    VirtualFree(ptr, 0, MEM_RELEASE);
}