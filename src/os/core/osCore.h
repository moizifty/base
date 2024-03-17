#ifndef OS_CORE_H
#define OS_CORE_H

#include "base\baseCoreTypes.h"

void* OSReserveMemory(u64 size);
void OSCommitMemory(void *ptr, u64 size);
void OSDecommitMemory(void *ptr, u64 size);
void OSFreeMemory(void *ptr, u64 size);

bool OSRunProcessEx(BaseArena *arena, str8 app, str8 args, void *peb, str8 *outStr, str8 *errStr);
#endif