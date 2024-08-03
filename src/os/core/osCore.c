#include "osCore.h"

#ifdef OS_WIN32
#include "win32\osCoreWin32.c"
#else
#error Platform not defined
#endif

global OSState *gOSState = null;

OSState *OSGetState(void)
{
    return gOSState;
}