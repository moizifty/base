#ifndef OS_H
#define OS_H

#ifndef baseArenaReserveImpl
#define baseArenaReserveImpl OSReserveMemory
#endif
#ifndef baseArenaCommitImpl
#define baseArenaCommitImpl OSCommitMemory
#endif
#ifndef baseArenaDecommitImpl
#define baseArenaDecommitImpl OSDecommitMemory
#endif
#ifndef baseArenaFreeImpl
#define baseArenaFreeImpl OSFreeMemory
#endif

#include "core\osCore.h"
#include "gfx\osGfx.h"

#endif