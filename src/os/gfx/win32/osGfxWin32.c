#include "..\osGfx.h"

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

OSHandle OSGfxOpenWindow(str8 title, i64 sizeX, i64 sizeY, i64 posX, i64 posY)
{
    // todo convert str8 into str16 for the window stuff
    // for now im using A version of function to get around this.
    i64 x = (posX < 0) ? CW_USEDEFAULT : posX;
    i64 y = (posY < 0) ? CW_USEDEFAULT : posY;

    i64 sx = (sizeX < 0) ? CW_USEDEFAULT : sizeX;
    i64 sy = (sizeY < 0) ? CW_USEDEFAULT : sizeY;

    HWND wnd = CreateWindowA(OS_GFX_WIN32_DEFAULT_CLASS_NAMEA, title.data, WS_OVERLAPPEDWINDOW, x, y, sx, sy, null, null, HINST_THIS, null);
    OSHandle h = (OSHandle){(u64)wnd};
    OSGfxFirstPaint(h);

    return h;
}

void OSGfxFirstPaint(OSHandle wnd)
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