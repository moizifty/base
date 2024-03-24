#include "osCore.h"

#ifdef OS_WINDOWS
#include "win32\osCoreWin32.c"
#else
#error Platform not defined
#endif