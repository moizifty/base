#ifndef OS_GFX_H
#define OS_GFX_H

#include "base\baseCore.h"
#include "base\baseMath.h"
#include "base\baseMemory.h"
#include "os\core\osCore.h"

#ifdef OS_WIN32
#include "win32\osGfxWin32.h"
#else
#error Platform not defined
#endif

typedef struct OSGfxState
{
    u8 _opaque;
}OSGfxState;

typedef enum OSEventKind
{
    OS_EVENT_WINDOW_CLOSE,
    OS_EVENT_WINDOW_LOST_FOCUS,
    OS_EVENT_KEY_PRESS,
    OS_EVENT_KEY_RELEASE,
    OS_EVENT_COUNT,
}OSEventKind;

typedef struct OSEvent
{
    struct OSEvent *next;
    struct OSEvent *prev;

    OSEventKind kind;
    
    union
    {
        OSKey key;
    };
}OSEvent;

typedef struct OSEventList
{
    OSEvent *first;
    OSEvent *last;
    u64 len;
}OSEventList;

OSGfxState *OSGfxInitEx(BaseArena *arena, void *extra);
OSGfxState *OSGfxInit(BaseArena *arena);
OSHandle OSGfxWindowOpen(str8 title, vec2i size, vec2i pos);
void OSGfxWindowFirstPaint(OSHandle wnd);

OSEventList OSGfxProcessEvents(BaseArena *arena);
bool OSGfxProcessInputEvents(BaseArena *arena);

// inputs
bool OSIsKeyHeld(OSKey key);
bool OSIsKeyPressed(OSKey key);
bool OSIsKeyReleased(OSKey key);
#endif