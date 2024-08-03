#include "osGfx.h"

#ifdef OS_WIN32
#include "win32\osGfxWin32.c"
#else
#error Platform not defined
#endif