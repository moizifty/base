#ifndef OS_GFX_H
#define OS_GFX_H

#include "base/baseCore.h"
#include "base/baseMath.h"
#include "base/baseMemory.h"
#include "os/core/osCore.h"

#if OS_WIN32
#include "win32/osGfxWin32.h"
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

OSGfxState *OSGfxInitEx(Arena *arena, void *extra);
OSGfxState *OSGfxInit(Arena *arena);
OSHandle OSGfxWindowOpen(str8 title, vec2i size, vec2i pos);
void OSGfxWindowFirstPaint(OSHandle wnd);

OSEventList OSGfxProcessEvents(Arena *arena);
bool OSGfxProcessInputEvents(Arena *arena);

// inputs
bool OSGfxIsKeyHeld(OSKey key);
bool OSGfxIsKeyPressed(OSKey key);
bool OSGfxIsKeyReleased(OSKey key);
#endif