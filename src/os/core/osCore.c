#include "osCore.h"

#ifdef OS_WIN32
#include "win32\osCoreWin32.c"
#else
#error Platform not defined
#endif

BASE_CREATE_EFFICIENT_LL_DEFS(OSHandleList, OSHandle);

global u64 gOSPerformanceFreq = 0;
global OSState *gOSState = null;

OSState *OSGetState(void)
{
    return gOSState;
}

bool OSHandleEquals(OSHandle a, OSHandle b)
{
    return a._u64 == b._u64;
}

u64 OSGetPerformanceFrequency(void)
{
    return gOSPerformanceFreq;
}
f64 OSConvertPerformanceCounterToMillisecondsF64(u64 counter)
{
    return (counter * 1000.0) / (f64)gOSPerformanceFreq;
}
