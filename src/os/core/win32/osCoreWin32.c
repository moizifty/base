#include "..\osCore.h"
#include "osCoreWin32.h"

#include "..\..\..\base\baseCore.h"
#include "..\..\..\base\baseStrings.h"
#include "..\..\..\base\baseMemory.h"
#include "..\..\..\base\baseThreads.h"
#include "log\log.h"

OSState *OSInit(BaseArena *arena)
{
    if (gOSState == null)
    {
        gOSState = baseArenaPush(arena, sizeof(OSState));
        // todo: set properly if need be
        gOSState->platformSpecific = null;
        gOSState->thisProcState = (OSProcessState)
        {
            // todo: maybe in future create a more generic
            // OSGetProcessPath, where it finds the path of any process
            // and if passed null it does this process.
            .binaryPath = OSGetProgramPath(arena),
            .logArena = baseArenaAlloc(BASE_GIGABYTES(2)),
            .logDirPath = OSGetProgramLogsDirectory(arena),
        };

        gOSState->thisProcState.processLog = logCreate(arena);
    }
    
    return gOSState;
}

void* OSReserveMemory(u64 size)
{
    return VirtualAlloc(NULL, size, MEM_RESERVE, PAGE_NOACCESS);
}
void OSCommitMemory(void *ptr, u64 size)
{
    VirtualAlloc(ptr, size, MEM_COMMIT, PAGE_READWRITE);
}
void OSDecommitMemory(void *ptr, u64 size)
{
    VirtualFree(ptr, size, MEM_DECOMMIT);
}
void OSFreeMemory(void *ptr, u64 size)
{
    BASE_UNUSED_PARAM(size);

    VirtualFree(ptr, 0, MEM_RELEASE);
}

void OSEnableVirtualTerminalSequenceProcessing(void)
{
    {
        HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
        DWORD consoleMode;
        GetConsoleMode(hConsole, &consoleMode);
        consoleMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING  ; //enable_virtual_terminal_processing

        SetConsoleMode(hConsole, consoleMode);
    }

    {
        HANDLE hConsole = GetStdHandle(STD_ERROR_HANDLE);
        DWORD consoleMode;
        GetConsoleMode(hConsole, &consoleMode);
        consoleMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING  ; //enable_virtual_terminal_processing

        SetConsoleMode(hConsole, consoleMode);
    }
}

// files
OSHandle OSFileOpen(str8 path, bool createLeadingDir, OSFileAccessFlags accessFlags, OSFileCreationKind creationKind)
{
    if (createLeadingDir)
    {
        str8 dirName = baseStringsStrChopPastLastSlash(path);

        if(!OSPathExists(dirName))
        {
            OSCreateDirectory(dirName, true);
        }
    }

    DWORD access = 0;
    DWORD share = 0;
    if(accessFlags & OS_FILEACCESS_READ)
    {
        access |= FILE_GENERIC_READ;
        share |= FILE_SHARE_READ;
    }

    if(accessFlags & OS_FILEACCESS_WRITE)
    {
        access |= FILE_GENERIC_WRITE;
        share |= FILE_SHARE_WRITE;
    }

    DWORD creation = 0;
    switch(creationKind)
    {
        case OS_FILECREATION_CREATE_NEW: creation = CREATE_NEW; break;
        case OOS_FILECREATION_CREATE_OVERRITE: creation = CREATE_ALWAYS; break;
        case OS_FILECREATION_OPEN_ALWAYS: creation = OPEN_ALWAYS; break;
        case OS_FILECREATION_OPEN_EXISTING: creation = OPEN_EXISTING; break;
    }

    HANDLE fileHandle = CreateFileA((char *)path.data, 
                                    access, 
                                    share, 
                                    NULL, 
                                    creation, 
                                    FILE_ATTRIBUTE_NORMAL,
                                    NULL);

    return (OSHandle){._u64 = (u64)fileHandle};
}
void OSFileWrite(OSHandle fileHandle, u8 *bytes, u64 numBytes)
{
    HANDLE h = (HANDLE) fileHandle._u64;
    WriteFile(h, 
              bytes,
              (int)numBytes,
              null,
              null);
}
void OSFileWriteStr8(OSHandle fileHandle, str8 str)
{
    OSFileWrite(fileHandle, str.data, str.len);
}
void OSFileWriteFmt(OSHandle fileHandle, char *fmt, ...)
{
    va_list va;
    va_start(va, fmt);

    BaseArenaTemp temp = baseTempBegin(null, 0);
    {
        str8 s = baseStringsPushStr8FmtV(temp.arena, fmt, va);

        OSFileWriteStr8(fileHandle, s);
    }
    baseTempEnd(temp);

    va_end(va);
}
U8Array OSFileReadAll(BaseArena *arena, str8 path)
{
    HANDLE fileHandle = CreateFileA((char *)path.data, 
                                    FILE_GENERIC_READ, 
                                    FILE_SHARE_READ, 
                                    NULL, 
                                    OPEN_EXISTING, 
                                    FILE_ATTRIBUTE_NORMAL,
                                    NULL);

    if (fileHandle == null || fileHandle == INVALID_HANDLE_VALUE)
    {
        return (U8Array){0};
    }

    u64 fileSize = OSGetFileSizeFromHandle((OSHandle){(u64)fileHandle});
    u8 *data = baseArenaPushNoZero(arena, fileSize);
    ReadFile(fileHandle, data, (u32)fileSize, null, null);
    CloseHandle(fileHandle);

    return (U8Array){.data = data, .len = fileSize};
}

void OSFileClose(OSHandle handle)
{
    CloseHandle((HANDLE)handle._u64);
}

bool OSPathExists(str8 path)
{
    DWORD dwAttrib = 0;
    BaseArenaTemp temp = baseTempBegin(null, 0);
    {
        str16 widePath = baseStr16FromFromStr8(temp.arena, path);
        dwAttrib = GetFileAttributes(widePath.data);
    }
    baseTempEnd(temp);

    return (dwAttrib != INVALID_FILE_ATTRIBUTES && 
          !(dwAttrib & FILE_ATTRIBUTE_DIRECTORY));
}
bool OSPathIsDirectory(str8 path)
{
    WIN32_FILE_ATTRIBUTE_DATA data = {0};
    GetFileAttributesExA((char *)path.data, GetFileExInfoStandard, &data);

    return data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY;
}
u64 OSGetFileSize(str8 path)
{
    WIN32_FILE_ATTRIBUTE_DATA data = {0};
    GetFileAttributesExA((char *)path.data, GetFileExInfoStandard, &data);

    u64 size = data.nFileSizeHigh;
    size = (size << 32) | data.nFileSizeLow;

    return size;
}
u64 OSGetFileSizeFromHandle(OSHandle handle)
{
    LARGE_INTEGER l = {0};
    GetFileSizeEx((HANDLE)handle._u64, &l);

    return l.QuadPart;
}
str8 OSGetFullPath(struct BaseArena *arena, str8 path)
{
    i8 buf[1];
    i64 needed = GetFullPathNameA((LPCSTR) path.data, 1, buf, null);

    str8 ret = {0};
    ret.data = baseArenaPush(arena, needed);
    ret.len = needed - 1;

    GetFullPathNameA((LPSTR) path.data, (DWORD) needed, (LPSTR) ret.data, null);
    return ret;
}

bool OSCreateDirectory(str8 path, bool createIntermediateDirs)
{
    bool result = true;
    BaseArenaTemp temp = baseTempBegin(null, 0);
    {
        str16 widePath = baseStr16FromFromStr8(temp.arena, path);

        if(createIntermediateDirs)
        {
            if (SHCreateDirectoryEx(null, widePath.data, null) != ERROR_SUCCESS)
            {
                result = false;
            }
        }
        else
        {
            return CreateDirectory(widePath.data, null);
        }
    }
    baseTempEnd(temp);

    return result;
}

OSFileAttributeFlags OSFileAttributesFromWin32(DWORD fileAttr)
{
    OSFileAttributeFlags flags = 0;
    if (fileAttr & FILE_ATTRIBUTE_DIRECTORY)
    {
        flags |= OS_FILEATTR_DIR;
    }

    return flags;
}
OSFileFindIter *OSFindFileBegin(struct BaseArena *arena, str8 path, OSFileFindOptionalParams *opt)
{
    OSFindFileIterWin32 *findFileData = baseArenaPush(arena, sizeof(OSFindFileIterWin32));
    OSFileFindIter *findIter = (OSFileFindIter *) findFileData;
    findFileData->optParams = (opt == null) ? (OSFileFindOptionalParams){0} : *opt;

    if (!baseStringsStrContains(path, '*'))
    {
        bool topLevel = findFileData->optParams.type == OS_FILEFIND_TYPE_TOP_LEVEL_DIR;

        if (path.len > 0)
        {
            str8 searchPath = path;
            if (topLevel)
            {
                searchPath = baseStringsPushStr8Fmt(arena, "%S\\*", path);
            }

            findFileData->handle = FindFirstFileA((i8 *) searchPath.data, &findFileData->findData);
            findFileData->originalPath = baseStringsPushStr8Fmt(arena, "%S", path);
        }
    }

    return findIter;
}

bool OSFindFileNext(struct BaseArena *arena, OSFileFindIter *iter, OSFileInfo *out)
{
    bool result = false;
    OSFindFileIterWin32 *win32Iter = (OSFindFileIterWin32 *) iter;
    if (win32Iter->handle == INVALID_HANDLE_VALUE || win32Iter->handle == null)
    {
        return false;
    }

    while(true)
    {
        bool fileNameInvalid = (win32Iter->findData.cFileName[0] == '.') && 
                                (win32Iter->findData.cFileName[1] == '\0' ||
                                (win32Iter->findData.cFileName[1] == '.'));

        if (!fileNameInvalid && !win32Iter->firstWasReturned)
        {
            result = true;
            win32Iter->firstWasReturned = true;
        }
        else
        {
            result = FindNextFileA(win32Iter->handle, &win32Iter->findData);
        }

        if (!result || !fileNameInvalid)
        {
            break;
        }
    }

    if (result)
    {
        bool topLevel = win32Iter->optParams.type == OS_FILEFIND_TYPE_TOP_LEVEL_DIR;
        if (topLevel)
        {
            out->path = baseStringsPushStr8Fmt(arena, "%S\\%s", win32Iter->originalPath, win32Iter->findData.cFileName);
        }
        else
        {
            out->path = baseStringsPushStr8Fmt(arena, "%S", win32Iter->originalPath);
        }

        out->attrs = OSFileAttributesFromWin32(win32Iter->findData.dwFileAttributes);
    }

    return result;
}
void OSFindFileEnd(OSFileFindIter *iter)
{
    OSFindFileIterWin32 *win32Iter = (OSFindFileIterWin32 *) iter;
    if (win32Iter->handle != INVALID_HANDLE_VALUE && win32Iter->handle != null)
    {
        FindClose(win32Iter->handle);
    }
}

//process
str8 OSGetProgramPath(BaseArena *arena)
{
    str8 ret = {0};
    BaseArenaTemp temp = baseTempBegin(&arena, 1);
    {
        u64 bufSize = BASE_KILOBYTES(4);
        u8 *buf = baseArenaPush(temp.arena, bufSize);
        GetModuleFileNameA(NULL, (i8 *) buf, (DWORD) bufSize);

        ret = baseStringsPushStr8Fmt(arena, "%s", buf);
    }

    baseTempEnd(temp);

    return ret;
}
str8 OSGetProgramDirectory(BaseArena *arena)
{
    str8 ret = OSGetProgramPath(arena);
    ret = baseStringsStrChopPastLastSlash(ret);

    return ret;
}
str8 OSGetProgramLogsDirectory(BaseArena *arena)
{
    str8 dir = OSGetProgramDirectory(arena);

    Str8List strs = {0};
    Str8ListPushLastFmt(arena, &strs, "%S\\%S\\",dir, LOG_FOLDER_NAME);

    return Str8ListJoin(arena, &strs, null);
}
bool OSRunProcessEx(BaseArena *arena, str8 app, str8 args, void *peb, str8 *outStr, str8 *errStr)
{
    SECURITY_ATTRIBUTES sa = 
    {
         .nLength = sizeof(SECURITY_ATTRIBUTES), 
         .lpSecurityDescriptor = NULL, 
         .bInheritHandle = true, 
    };

    HANDLE stdoutPread, stdoutPwrite;
    HANDLE stderrPread, stderrPwrite;

    // Create named pipes for stdout and stderr
    if (!CreatePipe(&stdoutPread, &stdoutPwrite, &sa, 0) ||
        !CreatePipe(&stderrPread, &stderrPwrite, &sa, 0)) 
    {
        return false;
    }

    SetHandleInformation(stdoutPread, HANDLE_FLAG_INHERIT, 0);
    SetHandleInformation(stderrPread, HANDLE_FLAG_INHERIT, 0);

    STARTUPINFOA si = 
    { 
         .cb = sizeof(STARTUPINFOA),
         .hStdOutput = stdoutPwrite,
         .hStdError = stderrPwrite,
         .dwFlags = STARTF_USESTDHANDLES,
    };

    // Launch the child process
    PROCESS_INFORMATION pi = {0};
    if (!CreateProcessA((LPSTR) app.data, (LPSTR) args.data, NULL, NULL, TRUE, 0, peb, NULL, &si, &pi)) 
    {
        return false;
    }
    
    // Close the read handles for the pipes
    CloseHandle(stdoutPwrite);
    CloseHandle(stderrPwrite);

    *outStr = STR8_LIT("");
    *errStr = STR8_LIT("");
    
    i64 totalStdoutSize = 0;
    i64 totalStderrSize = 0;

    Str8List outList = {0};
    Str8List errList = {0};
    
    DWORD exitCode = 0;

    while(true)
    {
        DWORD readSize = 0;

        GetExitCodeProcess(pi.hProcess, &exitCode);
        DWORD waitR = WaitForSingleObject(pi.hProcess, 0);

        bool isRunning = (waitR != WAIT_OBJECT_0) && (exitCode == STILL_ACTIVE);
        
        DWORD availBytes = 0;
        if((PeekNamedPipe(stdoutPread, NULL, 0, NULL, &availBytes, NULL) && availBytes))
        {
            char buf[255];

            if(!ReadFile(stdoutPread, buf, BASE_ARRAY_SIZE(buf), &readSize, NULL) || (readSize == 0))
            {
                break;
            }

            if(readSize > BASE_ARRAY_SIZE(buf))
            {

                str8 s = {0};
                s.data = baseArenaPushNoZero(arena, readSize + 1);
                s.len = readSize;

                ReadFile(stdoutPread, s.data, readSize, &readSize, NULL);

                Str8ListPushLast(arena, &outList, s);
            }
            else
            {
                Str8ListPushLast(arena, &outList, baseStr8((u8*)buf, readSize));
            }

            totalStdoutSize += readSize;
        }

        if(PeekNamedPipe(stderrPread, NULL, 0, NULL, &availBytes, NULL) && availBytes)
        {
            char buf[255];

            if(!ReadFile(stderrPread, buf, BASE_ARRAY_SIZE(buf), &readSize, NULL) || (readSize == 0))
            {
                break;
            }

            if(readSize > BASE_ARRAY_SIZE(buf))
            {

                str8 s = {0};
                s.data = baseArenaPushNoZero(arena, readSize + 1);
                s.len = readSize;

                ReadFile(stderrPread, s.data, readSize, &readSize, NULL);

                Str8ListPushLast(arena, &errList, s);
            }
            else
            {
                Str8ListPushLast(arena, &errList, baseStr8((u8*)buf, readSize));
            }

            totalStderrSize += readSize;
        }
        
        if(!isRunning && availBytes == 0)
        {
            break;
        }
    }
    

    if(totalStdoutSize > 0)
    {
        *outStr = Str8ListJoin(arena, &outList, null);
    }
    if(totalStderrSize > 0)
    {
        *errStr = Str8ListJoin(arena, &errList, null);
    }

    CloseHandle(stdoutPread);
    CloseHandle(stderrPread);

    // Wait for the child process to exit
    WaitForSingleObject(pi.hProcess, INFINITE);

    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);

    return true;
}

// Date and time
DateTime OSGetSytemTime(void)
{
    SYSTEMTIME sysTimeWin32;
    GetSystemTime(&sysTimeWin32);

    return (DateTime)
    {
        .year = (u16)sysTimeWin32.wYear,
        .month = (u8)sysTimeWin32.wMonth,
        .dayOfWeek = (u8)sysTimeWin32.wDayOfWeek + 1,
        .day = (u8)sysTimeWin32.wDay,
        .hour = (u8)sysTimeWin32.wHour,
        .min = (u8)sysTimeWin32.wMinute,
        .sec = (u8)sysTimeWin32.wSecond,
        .milli = (u16)sysTimeWin32.wMilliseconds,
    };
}
DateTime OSGetLocalTime(void)
{
    SYSTEMTIME sysTimeWin32;
    GetLocalTime(&sysTimeWin32);

    return (DateTime)
    {
        .year = (u16)sysTimeWin32.wYear,
        .month = (u8)sysTimeWin32.wMonth,
        .dayOfWeek = (u8)sysTimeWin32.wDayOfWeek + 1,
        .day = (u8)sysTimeWin32.wDay,
        .hour = (u8)sysTimeWin32.wHour,
        .min = (u8)sysTimeWin32.wMinute,
        .sec = (u8)sysTimeWin32.wSecond,
        .milli = (u16)sysTimeWin32.wMilliseconds,
    };
}

//other
range2i OSClientRectFromWindow(OSHandle handle)
{
    HWND wnd = (HWND)handle._u64;
    RECT r;
    GetClientRect(wnd, &r);

    return (range2i)
    {
        .topleft = Vec2i(r.left, r.top),
        .bottomright = Vec2i(r.right, r.bottom),
    };
}