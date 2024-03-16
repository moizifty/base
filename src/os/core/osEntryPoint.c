#if OS_WINDOWS
#include "win32\osEntryPointWin32.c"
#else
#error No entry point defined for current platform.
#endif