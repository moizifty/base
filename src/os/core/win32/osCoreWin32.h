#ifndef OS_CORE_WIN32_H
#define OS_CORE_WIN32_H

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <memoryapi.h>

#include "base\baseCoreTypes.h"

typedef struct OSFindFileIterWin32
{
    WIN32_FIND_DATAA findData;
    HANDLE handle;
    str8 originalPath;
    bool firstWasReturned;

    struct OSFileFindOptionalParams optParams;
}OSFindFileIterWin32;

#define HRFAILURE(HR)   ((HR) != S_OK)
EXTERN_C IMAGE_DOS_HEADER __ImageBase;
#define HINST_THIS ((HINSTANCE)&__ImageBase)

#endif