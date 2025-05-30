#include "osGfx.h"

#ifdef OS_WIN32
#include "win32\osGfxWin32.c"
#else
#error Platform not defined
#endif

global OSKeyState gOSGfxFrameKeyStates[OS_KEY_COUNT] = {0};
global OSKeyState gOSGfxPrevFrameKeyStates[OS_KEY_COUNT] = {0};

bool OSGfxIsKeyHeld(OSKey key)
{
    return gOSGfxFrameKeyStates[key].pressed;
}
bool OSGfxIsKeyPressed(OSKey key)
{
    return !gOSGfxPrevFrameKeyStates[key].pressed && gOSGfxFrameKeyStates[key].pressed;
}
bool OSGfxIsKeyReleased(OSKey key)
{
    return gOSGfxPrevFrameKeyStates[key].pressed && !gOSGfxFrameKeyStates[key].pressed;
}