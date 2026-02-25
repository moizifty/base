#pragma comment(lib, "user32.lib")
#pragma comment(lib, "kernel32.lib")

#ifndef OS_GFX_WIN32_H
#define OS_GFX_WIN32_H

#include "os/core/win32\osCoreWin32.h"

#define OS_GFX_WIN32_DEFAULT_CLASS_NAMEA "AppWindowClass"
#define OS_GFX_WIN32_DEFAULT_CLASS_NAMEW L"AppWindowClass"
#define OS_GFX_WIN32_DEFAULT_CLASS_NAME TEXT("AppWindowClass")

#define OS_GFX_WIN32_DEFAULT_CLASS  ((WNDCLASS) \
                                     { \
                                        .style = CS_HREDRAW | CS_VREDRAW, \
                                        .lpszMenuName = NULL, \
                                        .lpszClassName = OS_GFX_WIN32_DEFAULT_CLASS_NAME, \
                                        .lpfnWndProc = OSGfxWin32WindowProc, \
                                        .hInstance = HINST_THIS, \
                                    })

typedef struct OSGfxInitExtraDataWin32
{
    HWND wnd;
}OSGfxInitExtraDataWin32;

typedef struct OSGfxStateWin32
{
    HWND mainWnd;
}OSGfxStateWin32;

LRESULT OSGfxWin32WindowProc(HWND wnd, UINT msg, WPARAM wParam, LPARAM lParam);
#endif