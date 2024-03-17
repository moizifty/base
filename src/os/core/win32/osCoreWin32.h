#ifndef OS_CORE_WIN32_H
#define OS_CORE_WIN32_H

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <memoryapi.h>

#include "base\baseCoreTypes.h"

bool OSRunProcessEx(BaseArena *arena, str8 app, str8 args, void *peb, str8 *outStr, str8 *errStr);

#endif