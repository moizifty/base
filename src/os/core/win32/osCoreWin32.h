#pragma comment(lib, "shell32.lib")

#ifndef OS_CORE_WIN32_H
#define OS_CORE_WIN32_H

#define UNICODE
#define __UNICODE
#define __UNICODE__
#define _UNICODE_
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <memoryapi.h>

#include "base\baseCoreTypes.h"

#include <ShlObj.h>

typedef struct OSFindFileIterWin32
{
    WIN32_FIND_DATAA findData;
    HANDLE handle;
    str8 originalPath;
    bool firstWasReturned;

    struct OSFileFindOptionalParams optParams;
}OSFindFileIterWin32;

#define HRFAILURE(HR)   ((HR) != S_OK)
global IMAGE_DOS_HEADER __ImageBase;
#define HINST_THIS ((HINSTANCE)&__ImageBase)

#endif