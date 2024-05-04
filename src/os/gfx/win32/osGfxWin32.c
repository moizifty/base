#include "..\osGfx.h"

#include "base\baseStrings.h"
#include "base\baseThreads.h"
#include "osGfxWin32.h"

threadlocal BaseArena *gOSWin32TLEventsArena = null;
threadlocal OSEventList gOSWin32TLEvents = {0};

OSGfxState *OSGfxInitEx(BaseArena *arena, void *extra)
{
    UNREFERENCED_PARAMETER(extra);

    OSGfxStateWin32 *gfxState = baseArenaPush(arena, sizeof(OSGfxStateWin32));
    gOSWin32TLEventsArena = arena;

    return (OSGfxState *) gfxState;
}
OSGfxState *OSGfxInit(BaseArena *arena)
{
    WNDCLASS class = OS_GFX_WIN32_DEFAULT_CLASS;

    if(RegisterClass(&class) == 0)
    {
        return null;
    }

    return OSGfxInitEx(arena, null);
}

OSHandle OSGfxWindowOpen(str8 title, vec2i size, vec2i pos)
{
    i64 x = (i64) ((pos.x < 0) ? CW_USEDEFAULT : pos.x);
    i64 y = (i64) ((pos.y < 0) ? CW_USEDEFAULT : pos.y);

    i64 sx = (i64) ((size.x < 0) ? CW_USEDEFAULT : size.x);
    i64 sy = (i64) ((size.y < 0) ? CW_USEDEFAULT : size.y);

    OSHandle h = {0};
    BaseArenaTemp temp = baseTempBegin(null, 0);
    {
        str16 str16 = baseStr16FromFromStr8(temp.arena, title);
        HWND wnd = CreateWindow(OS_GFX_WIN32_DEFAULT_CLASS_NAME, str16.data, WS_OVERLAPPEDWINDOW, (int)x, (int)y, (int)sx, (int)sy, null, null, HINST_THIS, null);
        h = (OSHandle){(u64)wnd};
    }

    baseTempEnd(temp);

    return h;
}

void OSGfxWindowFirstPaint(OSHandle wnd)
{
    HWND wndHandle = (HWND)wnd._u64;
    ShowWindow(wndHandle, SW_SHOW);
    UpdateWindow(wndHandle);
}

OSEventList OSGfxProcessEvents(BaseArena *arena)
{
    UNREFERENCED_PARAMETER(arena);

    for(MSG msg; PeekMessage(&msg, null, 0, 0, PM_REMOVE);)
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return gOSWin32TLEvents;
}

LRESULT OSGfxWin32WindowProc(HWND wnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    OSEvent *event = null;
    LRESULT result = 0;

    switch (msg)
    {
        case WM_CLOSE:
        {
            event = baseArenaPush(gOSWin32TLEventsArena, sizeof(OSEvent));
            event->kind = OS_EVENT_WINDOW_CLOSE;

            result = 0;
        }break;

        default:
        {
            result = DefWindowProc(wnd, msg, wParam, lParam);
        }break;
    }

    if(event != null)
    {
        BaseDllNodePushLast(gOSWin32TLEvents.first, gOSWin32TLEvents.last, event);
        gOSWin32TLEvents.len += 1;
    }   

    return result;
}