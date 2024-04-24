#include "osGfx.h"

#ifdef OS_WINDOWS
#include "win32\osGfxWin32.c"
#else
#error Platform not defined
#endif