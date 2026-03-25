#include "..\osCore.h"
#include "osCoreWin32.h"

#include "..\..\..\base\baseCore.h"
#include "..\..\..\base\baseStrings.h"
#include "..\..\..\base\baseMemory.h"
#include "..\..\..\base\baseThreads.h"
#include "base/baseLog.h"

global OSKey gOSWin32VKToOSKeyTable[256] = 
{
    ['A'] = OS_KEY_A,
    ['B'] = OS_KEY_B,
    ['C'] = OS_KEY_C,
    ['D'] = OS_KEY_D,
    ['E'] = OS_KEY_E,
    ['F'] = OS_KEY_F,
    ['G'] = OS_KEY_G,
    ['H'] = OS_KEY_H,
    ['I'] = OS_KEY_I,
    ['J'] = OS_KEY_J,
    ['K'] = OS_KEY_K,
    ['L'] = OS_KEY_L,
    ['M'] = OS_KEY_M,
    ['N'] = OS_KEY_N,
    ['O'] = OS_KEY_O,
    ['P'] = OS_KEY_P,
    ['Q'] = OS_KEY_Q,
    ['R'] = OS_KEY_R,
    ['S'] = OS_KEY_S,
    ['T'] = OS_KEY_T,
    ['U'] = OS_KEY_U,
    ['V'] = OS_KEY_V,
    ['W'] = OS_KEY_W,
    ['X'] = OS_KEY_X,
    ['Y'] = OS_KEY_Y,
    ['Z'] = OS_KEY_Z,
};

u64 gOSWin32OSKeyToVKTable[OS_KEY_COUNT] = 
{
    [OS_KEY_A] = 'A',
    [OS_KEY_B] = 'B',
    [OS_KEY_C] = 'C',
    [OS_KEY_D] = 'D',
    [OS_KEY_E] = 'E',
    [OS_KEY_F] = 'F',
    [OS_KEY_G] = 'G',
    [OS_KEY_H] = 'H',
    [OS_KEY_I] = 'I',
    [OS_KEY_J] = 'J',
    [OS_KEY_K] = 'K',
    [OS_KEY_L] = 'L',
    [OS_KEY_M] = 'M',
    [OS_KEY_N] = 'N',
    [OS_KEY_O] = 'O',
    [OS_KEY_P] = 'P',
    [OS_KEY_Q] = 'Q',
    [OS_KEY_R] = 'R',
    [OS_KEY_S] = 'S',
    [OS_KEY_T] = 'T',
    [OS_KEY_U] = 'U',
    [OS_KEY_V] = 'V',
    [OS_KEY_W] = 'W',
    [OS_KEY_X] = 'X',
    [OS_KEY_Y] = 'Y',
    [OS_KEY_Z] = 'Z',
};
OSState *OSInit(Arena *arena)
{
    if (gOSState == null)
    {
        gOSState = arenaPush(arena, sizeof(OSState));
        // todo: set properly if need be
        gOSState->platformSpecific = null;
        gOSState->thisProcState = (OSProcessState)
        {
            // todo: maybe in future create a more generic
            // OSGetProcessPath, where it finds the path of any process
            // and if passed null it does this process.
            .binaryPath = OSGetProgramPath(arena),
            .logDirPath = OSGetProgramLogsDirectory(arena),
            //.logArena = arenaAlloc(BASE_GIGABYTES(2)),
        };

        //gOSState->thisProcState.processLog = logCreate(arena);
    }
    
    return gOSState;
}

bool OSIsHandleValid(OSHandle handle)
{
    return handle._u64 != (u64)null && handle._u64 != (u64)INVALID_HANDLE_VALUE;
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
bool OSStdoutIsRedirected(void)
{
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    DWORD consoleMode;
    return GetConsoleMode(hConsole, &consoleMode);
}

// files
OSHandle OSFileOpen(str8 path, bool createLeadingDir, OSFileAccessFlags accessFlags, OSFileCreationKind creationKind)
{
    if (createLeadingDir)
    {
        str8 dirName = Str8ChopPastLastSlash(path);

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
        case OS_FILECREATION_CREATE_OVERRITE: creation = CREATE_ALWAYS; break;
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
void OSFileWriteU32(OSHandle fileHandle, u32 n)
{
    OSFileWrite(fileHandle, (u8*)&n, sizeof(n));
}
void OSFileWriteU64(OSHandle fileHandle, u64 n)
{
    OSFileWrite(fileHandle, (u8*)&n, sizeof(n));
}


void OSFileWriteFmt(OSHandle fileHandle, char *fmt, ...)
{
    va_list va;
    va_start(va, fmt);

    ArenaTemp temp = baseTempBegin(null, 0);
    {
        str8 s = Str8PushFmtV(temp.arena, fmt, va);

        OSFileWriteStr8(fileHandle, s);
    }
    baseTempEnd(temp);

    va_end(va);
}
U8Array OSFileReadAll(Arena *arena, str8 path)
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
    u8 *data = arenaPushNoZero(arena, fileSize);
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
    ArenaTemp temp = baseTempBegin(null, 0);
    {
        str16 widePath = Str16FromFromStr8(temp.arena, path);
        dwAttrib = GetFileAttributes(widePath.data);
    }
    baseTempEnd(temp);

    return (dwAttrib != INVALID_FILE_ATTRIBUTES);
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
str8 OSGetFullPath(struct Arena *arena, str8 path)
{
    i8 buf[1];
    i64 needed = GetFullPathNameA((LPCSTR) path.data, 1, buf, null);

    str8 ret = {0};
    ret.data = arenaPush(arena, needed);
    ret.len = needed - 1;

    GetFullPathNameA((LPSTR) path.data, (DWORD) needed, (LPSTR) ret.data, null);
    return ret;
}

bool OSFileDelete(str8 path)
{
    ArenaTemp temp = baseTempBegin(null, 0);

    str16 wide = Str16FromFromStr8(temp.arena, path);
    bool result = DeleteFileW((LPCWSTR)wide.data);

    baseTempEnd(temp);

    return result;
}
bool OSDirectoryDelete(str8 path, bool recursive)
{
    ArenaTemp temp = baseTempBegin(null, 0);
    bool result = true;
    if (recursive)
    {
        Str8List allPaths = {0};
        typedef struct FindTask
        {
            str8 path;

            struct FindTask *next;
            struct FindTask *prev;
        }FindTask;

        FindTask initialTask = {.path = Str8PushFmt(temp.arena, "%S", path)};

        FindTask *firstTask = &initialTask;
        FindTask *lastTask = &initialTask;

        for(FindTask *task = firstTask; task != null; task = task->next)
        {
            OSFileFindIter *iter = OSFindFileBegin(temp.arena, Str8PushFmt(temp.arena, "%S\\*", task->path), null);
            if (iter != null)
            {
                for(OSFileInfo fileInfo = {0}; OSFindFileNext(temp.arena, iter, &fileInfo); )
                {
                    if (fileInfo.attrs & OS_FILEATTR_DIR)
                    {
                        FindTask *t = arenaPushType(temp.arena, FindTask);
                        t->path = Str8PushFmt(temp.arena, "%S\\%S", task->path, fileInfo.name);

                        BaseDllNodePushLast(firstTask, lastTask, t);
                        Str8ListPushLast(temp.arena, &allPaths, Str8PushFmt(temp.arena, "%S\\%S", task->path, fileInfo.name));
                    }
                    else
                    {
                        Str8ListPushFirst(temp.arena, &allPaths, Str8PushFmt(temp.arena, "%S\\%S", task->path, fileInfo.name));
                    }
                }

                OSFindFileEnd(iter);
            }
        }

        BASE_LIST_FOREACH(Str8ListNode, pathNode, allPaths)
        {
            result = result && OSPathDelete(pathNode->val, recursive);
        }
    }

    str16 widePath = Str16FromFromStr8(temp.arena, path);
    result = result && RemoveDirectory((LPCWSTR)widePath.data);
    baseTempEnd(temp);

    return result;
}
bool OSPathDelete(str8 path, bool recursive)
{
    if (OSPathIsDirectory(path))
    {
        return OSDirectoryDelete(path, recursive);
    }
    else
    {
        return OSFileDelete(path);
    }
}

bool OSCreateDirectory(str8 path, bool createIntermediateDirs)
{
    bool result = true;
    ArenaTemp temp = baseTempBegin(null, 0);
    {
        str16 widePath = Str16FromFromStr8(temp.arena, path);

        if(createIntermediateDirs)
        {
            Str8List pathSplit = Str8Split(temp.arena, path, STR8_LIT("/"), STR_MATCHFLAGS_SLASH_INSENSITIVE, STR_SPLITFLAGS_DISCARD_EMPTY);
            path = STR8_EMPTY;
            
            BASE_LIST_FOREACH(Str8ListNode, node, pathSplit)
            {
                if (pathSplit.first == node)
                {
                    path = node->val;
                }
                else
                {
                    path = Str8PushFmt(temp.arena, "%S/%S", path, node->val);
                }

                widePath = Str16FromFromStr8(temp.arena, path);
                if (!CreateDirectory(widePath.data, null))
                {
                    if (GetLastError() != ERROR_ALREADY_EXISTS)
                    {
                        result = false;
                        break;
                    }
                }
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
OSFileFindIter *OSFindFileBegin(struct Arena *arena, str8 path, OSFileFindOptionalParams *opt)
{
    OSFindFileIterWin32 *findFileData = arenaPush(arena, sizeof(OSFindFileIterWin32));
    OSFileFindIter *findIter = (OSFileFindIter *) findFileData;
    findFileData->optParams = (opt == null) ? (OSFileFindOptionalParams){0} : *opt;

    if (path.len > 0)
    {
        findFileData->handle = FindFirstFileA((i8 *) path.data, &findFileData->findData);
    }
    return findIter;
}

bool OSFindFileNext(struct Arena *arena, OSFileFindIter *iter, OSFileInfo *out)
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
        out->name = Str8PushFmt(arena, "%s", win32Iter->findData.cFileName);
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

Str8List OSGetFilePaths(Arena *arena, str8 dir, str8 pattern, bool recursive)
{
    Str8List ret = {0};

    str8 searchPath = (recursive) ? Str8PushFmt(arena, "%S\\*", dir) : Str8PushFmt(arena, "%S\\%S", dir, pattern);

    OSFileFindIter *findIter = OSFindFileBegin(arena, searchPath, null);
    for(OSFileInfo fileInfo = {0}; OSFindFileNext(arena, findIter, &fileInfo); )
    {
        if (fileInfo.attrs & OS_FILEATTR_DIR)
        {
            if (recursive)
            {
                str8 subDirPath = Str8PushFmt(arena, "%S\\%S", dir, fileInfo.name);
                Str8List subDirList = OSGetFilePaths(arena, subDirPath, pattern, recursive);

                Str8ListPushListLast(arena, &ret, &subDirList);
            }
        }
        else
        {
            if (recursive)
            {
                if (PathMatchSpecA((i8*) fileInfo.name.data, (i8*) pattern.data))
                {
                    Str8ListPushLastFmt(arena, &ret, "%S\\%S", dir, fileInfo.name);
                }
            }
            else 
            {
                Str8ListPushLastFmt(arena, &ret, "%S\\%S", dir, fileInfo.name);
            }
        }
    }

    OSFindFileEnd(findIter);

    return ret;
}

//process
str8 OSGetProgramPath(Arena *arena)
{
    str8 ret = {0};
    ArenaTemp temp = baseTempBegin(&arena, 1);
    {
        u64 bufSize = BASE_KILOBYTES(4);
        u8 *buf = arenaPush(temp.arena, bufSize);
        GetModuleFileNameA(NULL, (i8 *) buf, (DWORD) bufSize);

        ret = Str8PushFmt(arena, "%s", buf);
    }

    baseTempEnd(temp);

    return ret;
}
str8 OSGetProgramDirectory(Arena *arena)
{
    str8 ret = OSGetProgramPath(arena);
    ret = Str8ChopPastLastSlash(ret);

    return ret;
}
str8 OSGetProgramLogsDirectory(Arena *arena)
{
    str8 dir = OSGetProgramDirectory(arena);

    Str8List strs = {0};
    Str8ListPushLastFmt(arena, &strs, "%S\\%S\\",dir, LOG_FOLDER_NAME);

    return Str8ListJoin(arena, &strs, null);
}

OSHandle OSProcessOpen(struct Arena *arena, str8 app, str8 args, void *environment)
{
    OSHandle handle = {._u64 = (u64)INVALID_HANDLE_VALUE};

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
        return handle;
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
    if (CreateProcessA((app.len == 0) ? null : (LPSTR) app.data, args.len == 0 ? null : (LPSTR) args.data, NULL, NULL, TRUE, 0, environment, NULL, &si, &pi)) 
    {
        OSProcessWin32 *proc = arenaPushType(arena, OSProcessWin32);
        proc->procInfo = pi;
        proc->stdoutReadPipe = stdoutPread;
        proc->stderrReadPipe = stderrPread;
        proc->running = true;

        handle._u64 = (u64)proc;
    }
    
    // Close the read handles for the pipes
    CloseHandle(stdoutPwrite);
    CloseHandle(stderrPwrite);

    return handle;
}
void OSProcessWait(OSHandle procHandle)
{
    if (OSIsHandleValid(procHandle))
    {
        OSProcessWin32 *proc = (OSProcessWin32*)procHandle._u64;
        WaitForSingleObject(proc->procInfo.hProcess, INFINITE);
    }
}
void OSProcessClose(OSHandle procHandle)
{
    if (OSIsHandleValid(procHandle))
    {
        OSProcessWin32 *proc = (OSProcessWin32*)procHandle._u64;

        if (proc->stderrReadPipe != INVALID_HANDLE_VALUE &&
            proc->stdoutReadPipe != INVALID_HANDLE_VALUE)
        {
            CloseHandle(proc->stderrReadPipe);
            CloseHandle(proc->stdoutReadPipe);
        }

        CloseHandle(proc->procInfo.hProcess);
        CloseHandle(proc->procInfo.hThread);
    }
}

bool OSProcessReadStdoutStderr(struct Arena *arena, OSHandle procHandle, str8 *stdoutStr, str8 *stderrStr)
{
    *stdoutStr = STR8_LIT("");
    *stderrStr = STR8_LIT("");

    if (OSIsHandleValid(procHandle))
    {
        OSProcessWin32 *proc = (OSProcessWin32*)procHandle._u64;
        if (proc->running)
        {
            DWORD exitCode = 0;

            GetExitCodeProcess(proc->procInfo.hProcess, &exitCode);
            DWORD waitResult = WaitForSingleObject(proc->procInfo.hProcess, 0);

            proc->running = (waitResult != WAIT_OBJECT_0) && (exitCode == STILL_ACTIVE);
        }

        DWORD readSize = 0;
        DWORD availBytes = 0;

        if((PeekNamedPipe(proc->stdoutReadPipe, NULL, 0, NULL, &availBytes, NULL) && availBytes))
        {
            stdoutStr->data = arenaPushNoZero(arena, availBytes + 1);
            stdoutStr->len = availBytes;

            if(!ReadFile(proc->stdoutReadPipe, stdoutStr->data, (DWORD)stdoutStr->len, &readSize, NULL) || (readSize == 0))
            {
                return false;
            }
        }

        if((PeekNamedPipe(proc->stderrReadPipe, NULL, 0, NULL, &availBytes, NULL) && availBytes))
        {
            stderrStr->data = arenaPushNoZero(arena, availBytes + 1);
            stderrStr->len = availBytes;

            if(!ReadFile(proc->stderrReadPipe, stderrStr->data, (DWORD) stderrStr->len, &readSize, NULL) || (readSize == 0))
            {
                return false;
            }
        }

        if (!proc->running)
        {
            CloseHandle(proc->stderrReadPipe);
            CloseHandle(proc->stdoutReadPipe);

            proc->stdoutReadPipe = proc->stderrReadPipe = INVALID_HANDLE_VALUE;
            return false;
        }

        return true;
    }

    return false;
}

OSHandle OSLoadDynamicLibrary(str8 name)
{
    OSHandle handle = {._u64 = 0};

    ArenaTemp temp = baseTempBegin(null, 0);
    {
        str16 name16 = Str16FromFromStr8(temp.arena, name);
        handle._u64 = (u64) LoadLibraryW(name16.data);
    }
    baseTempEnd(temp);

    return handle;
}

void *OSGetExportAddressFromDynamicLibrary(OSHandle dynLib, str8 name)
{
    void *ptr = null;

    ArenaTemp temp = baseTempBegin(null, 0);
    {
        // needs '\0' at end
        str8 nameCopied = Str8PushCopy(temp.arena, name);
        FARPROC fnptr = GetProcAddress((HMODULE) dynLib._u64, (i8*)nameCopied.data);

        ptr = (void*)fnptr;
    }
    baseTempEnd(temp);

    return ptr;
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

u64 OSGetPerformanceCounter(void)
{
    u64 counter = 0;
    QueryPerformanceCounter((LARGE_INTEGER *)&counter);
    
    return counter;
}

//env
str8 OSGetEnvironmentVar(Arena *arena, str8 var)
{
    str8 val = STR8_LIT("");

    ArenaTemp temp = baseTempBegin(&arena, 1);
    var = Str8PushFmt(arena, "%S", var);
    
    i64 size = GetEnvironmentVariableA((i8*)var.data, null, 0);
    if(size > 0)
    {
        val.data = arenaPushArray(arena, u8, size);
        GetEnvironmentVariableA((i8*)var.data, (i8*)val.data, (DWORD)size);

        val.len = size - 1;
    }

    baseTempEnd(temp);
    return val;
}

//other
vec2i OSScreenCoordToClientCoord(OSHandle wndHandle, vec2i screen)
{
    HWND wnd = (HWND)wndHandle._u64;
    POINT p = {.x = (LONG)screen.x, .y = (LONG)screen.y};
    ScreenToClient(wnd, &p);

    return Vec2i(p.x, p.y);
}
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

vec2i OSGetCursorScreenCoordPos()
{
    POINT p;
    GetCursorPos(&p);
    
    return Vec2i(p.x, p.y);
}
vec2i OSGetCursorClientCoordPos(OSHandle wndHandle)
{
    vec2i v = OSGetCursorScreenCoordPos();
    return OSScreenCoordToClientCoord(wndHandle, v);
}

OSHandle OSGetCurrentThread()
{
    OSHandle handle = {._u64 = (u64)GetCurrentThread()};

    return handle;
}

// this will not set the threadname on the threads context
// use the function in basethreads for that
void OSSetThreadDebuggerName(OSHandle thread, str8 name)
{
    HANDLE threadHandle = (HANDLE)thread._u64;

    ArenaTemp temp = baseTempBegin(null, 0);
    {
        str16 name16 = Str16FromFromStr8(temp.arena, name);
        SetThreadDescription(threadHandle, name16.data);
    }

    baseTempEnd(temp);
}