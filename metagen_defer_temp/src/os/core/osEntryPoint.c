#if OS_WIN32
#include "win32/osEntryPointWin32.c"
#elif OS_LINUX == 1
#include "linux/osEntryPointLinux.c"
#else
#error Current platform is not supported
#endif