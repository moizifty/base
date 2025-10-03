#include "..\osGfx.h"

#include "base\baseStrings.h"
#include "base\baseThreads.h"
#include "osGfxWin32.h"

threadlocal Arena *gOSWin32TLEventsArena = null;
threadlocal OSEventList gOSWin32TLEvents = {0};

global OSKey gOSWin32VKToOSKeyTable[256];

global OSKeyState gOSGfxFrameKeyStates[OS_KEY_COUNT];
global OSKeyState gOSGfxPrevFrameKeyStates[OS_KEY_COUNT];

OSGfxState *OSGfxInitEx(Arena *arena, void *extra)
{
    UNREFERENCED_PARAMETER(extra);

    OSGfxStateWin32 *gfxState = arenaPush(arena, sizeof(OSGfxStateWin32));
    gOSWin32TLEventsArena = arena;

    return (OSGfxState *) gfxState;
}
OSGfxState *OSGfxInit(Arena *arena)
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
    ArenaTemp temp = baseTempBegin(null, 0);
    {
        str16 str16 = Str16FromFromStr8(temp.arena, title);
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

OSEventList OSGfxProcessEvents(Arena *arena)
{
    UNREFERENCED_PARAMETER(arena);

    for(MSG msg; PeekMessage(&msg, null, 0, 0, PM_REMOVE);)
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return gOSWin32TLEvents;
}

bool OSGfxProcessInputEvents(Arena *arena)
{
    BASE_UNUSED_PARAM(arena);

    for(u64 i = 0; i < BASE_ARRAY_SIZE(gOSGfxFrameKeyStates); i++)
    {
        gOSGfxPrevFrameKeyStates[i] = gOSGfxFrameKeyStates[i];
    }

    BASE_LIST_FOREACH(OSEvent, event, gOSWin32TLEvents)
    {
        switch(event->kind)
        {
            case OS_EVENT_WINDOW_CLOSE:
            {
                return true;
            }break;

            case OS_EVENT_WINDOW_LOST_FOCUS:
            {
                // if the window loses focus, it loses input focus too
                // so we have to reset the keystates

                BASE_MEMSET(gOSGfxFrameKeyStates, 0, sizeof(gOSGfxFrameKeyStates));
                BaseListNodeRemove(gOSWin32TLEvents, event);
            }break;

            case OS_EVENT_KEY_PRESS: 
            case OS_EVENT_KEY_RELEASE:
            {
                gOSGfxFrameKeyStates[event->key].pressed = (event->kind == OS_EVENT_KEY_PRESS) ? true : false;

                BaseListNodeRemove(gOSWin32TLEvents, event);
            }break;
        }
    }

    return false;
}

LRESULT OSGfxWin32WindowProc(HWND wnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    OSEvent *event = null;
    LRESULT result = 0;

    switch (msg)
    {
        case WM_CLOSE:
        {
            event = arenaPush(gOSWin32TLEventsArena, sizeof(OSEvent));
            event->kind = OS_EVENT_WINDOW_CLOSE;

            result = 0;
        }break;
        
        case WM_KILLFOCUS:
        {
            event = arenaPush(gOSWin32TLEventsArena, sizeof(OSEvent));
            event->kind = OS_EVENT_WINDOW_LOST_FOCUS;

            result = 0;
        }break;

        case WM_KEYUP:
        case WM_KEYDOWN:
        {
            bool wasDown = (lParam & (1 << 30));
            bool isUp = (lParam & (1 << 31));
            
            // dont allow repeat events to be spammed
            if (!wasDown || isUp)
            {
                OSKey key = OS_KEY_NULL;

                if (wParam < BASE_ARRAY_SIZE(gOSWin32VKToOSKeyTable))
                {
                    key = gOSWin32VKToOSKeyTable[wParam];
                }

                event = arenaPush(gOSWin32TLEventsArena, sizeof(OSEvent));
                event->kind = (isUp) ? OS_EVENT_KEY_RELEASE : OS_EVENT_KEY_PRESS;
                event->key = key;
            }
            
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