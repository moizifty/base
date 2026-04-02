#include "osCore.h"

#if OS_WIN32
#include "win32/osCoreWin32.c"
#elif OS_LINUX == 1
#include "linux/osCoreLinux.c"
#else
#error Platform not defined
#endif

BASE_CREATE_EFFICIENT_LL_DEFS(OSHandleList, OSHandle)

u64 gOSPerformanceFreq = 0;
OSState *gOSState = null;

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
    return ((f64)counter * 1000.0) / (f64)gOSPerformanceFreq;
}

bool OSFileWriteAll(str8 path, U8Array bytes, bool createLeadingDir, bool overwrite)
{
    OSHandle handle = OSFileOpen(path, createLeadingDir, OS_FILEACCESS_WRITE, (overwrite) ? OS_FILECREATION_CREATE_OVERRITE : OS_FILECREATION_CREATE_NEW);
    OSFileWrite(handle, bytes.data, bytes.len);

    return true;
}
bool OSFileWriteAllStr8(str8 path, str8 str, bool createLeadingDir, bool overwrite)
{
    return OSFileWriteAll(path, (U8Array){.data = str.data, .len = str.len}, createLeadingDir, overwrite);
}