#pragma comment(lib, "shell32.lib")
#pragma comment(lib, "shlwapi.lib")
#pragma comment(lib, "user32.lib")
#pragma comment(lib, "gdi32.lib")
#pragma comment(lib, "kernel32.lib")

#ifndef OS_CORE_WIN32_H
#define OS_CORE_WIN32_H

#define UNICODE
#define __UNICODE
#define __UNICODE__
#define _UNICODE_
#define WIN32_LEAN_AND_MEAN
#define COBJMACROS
#include <windows.h>
#include <memoryapi.h>
#include <Shlwapi.h>

#include "base/baseCoreTypes.h"

#include <ShlObj.h>

typedef struct OSFindFileIterWin32
{
    WIN32_FIND_DATAA findData;
    HANDLE handle;
    bool firstWasReturned;

    struct OSFileFindOptionalParams optParams;
}OSFindFileIterWin32;

typedef struct OSProcessWin32
{
    PROCESS_INFORMATION procInfo;
    HANDLE stderrReadPipe;
    HANDLE stdoutReadPipe;

    bool running;
}OSProcessWin32;

#define HRFAILURE(HR)   ((HR) != S_OK && (HR) != S_FALSE)
global IMAGE_DOS_HEADER __ImageBase;
#define HINST_THIS ((HINSTANCE)&__ImageBase)
#define OS_CORE_WIN32_HINST HINST_THIS

OSFileAttributeFlags OSFileAttributesFromWin32(DWORD fileAttr);
DWORD Win32FileAttributesFromOSFileAttributes(OSFileAttributeFlags fileAttr);
#endif