#ifndef OS_GFX_H
#define OS_GFX_H

#include "base\baseCore.h"
#include "base\baseMath.h"
#include "base\baseMemory.h"
#include "os\core\osCore.h"

#ifdef OS_WINDOWS
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
    OS_EVENT_COUNT,
}OSEventKind;

typedef struct OSEvent
{
    struct OSEvent *next;
    struct OSEvent *prev;

    OSEventKind kind;
}OSEvent;

typedef struct OSEventList
{
    OSEvent *first;
    OSEvent *last;
    u64 len;
}OSEventList;

OSGfxState *OSGfxInitEx(BaseArena *arena, void *extra);
OSGfxState *OSGfxInit(BaseArena *arena);
OSHandle OSGfxOpenWindow(str8 title, vec2f size, vec2f pos);
void OSGfxFirstPaint(OSHandle wnd);

OSEventList OSGfxProcessEvents(BaseArena *arena);
#endif