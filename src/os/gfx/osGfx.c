#include "osGfx.h"

#ifdef OS_WIN32
#include "win32\osGfxWin32.c"
#else
#error Platform not defined
#endif

global OSKeyState gOSKeyStates[OS_KEY_COUNT] = {0};
global OSKeyState gOSPrevKeyStates[OS_KEY_COUNT] = {0};

bool OSIsKeyHeld(OSKey key)
{
    return gOSKeyStates[key].pressed;
}
bool OSIsKeyPressed(OSKey key)
{
    return !gOSPrevKeyStates[key].pressed && gOSKeyStates[key].pressed;
}
bool OSIsKeyReleased(OSKey key)
{
    return gOSPrevKeyStates[key].pressed && !gOSKeyStates[key].pressed;
}