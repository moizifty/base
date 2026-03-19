#include "../osCore.h"

#include "osCoreLinux.h"
#include "base/baseStrings.h"
#include "base/baseMemory.h"
#include "base/baseThreads.h"

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

void* OSReserveMemory(u64 size)
{
    return mmap(NULL, size, PROT_NONE, MAP_ANONYMOUS | MAP_PRIVATE, -1, 0);
}
void OSCommitMemory(void *ptr, u64 size)
{
    mprotect(ptr, size, PROT_READ | PROT_WRITE);
}
void OSDecommitMemory(void *ptr, u64 size)
{
    madvise(ptr, size, MADV_DONTNEED);
    mprotect(ptr, size, PROT_NONE);
}
void OSFreeMemory(void *ptr, u64 size)
{
    munmap(ptr, size);
}

void OSEnableVirtualTerminalSequenceProcessing(void)
{
    // do nothing, already enabled on linux
}

str8 OSGetProgramPath(Arena *arena)
{
    str8 pathToRet = {0};
    ArenaTemp temp = baseTempBegin(&arena, 1);
    {
        str8 path = STR8_LIT("");
        int numRead = 0;
        while (!OSPathExists(path))
        {
            path = (str8){.data = arenaPushArray(temp.arena, u8, path.len + 255), .len = path.len + 255};
            numRead = readlink("/proc/self/exe", (char*)path.data, path.len);
            path.data[numRead] = '\0';
            path.len = numRead;
        }
        
        pathToRet = Str8PushFmt(arena, "%S", path);
    }
    baseTempEnd(temp);

    return pathToRet;
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
    Str8ListPushLastFmt(arena, &strs, "%S/%S/", dir, LOG_FOLDER_NAME);

    return Str8ListJoin(arena, &strs, null);
}
u64 OSGetFileSizeFromHandle(OSHandle handle)
{
    int fd = (int)handle._u64;

    struct stat statBuf;
    fstat(fd, &statBuf);

    return statBuf.st_size;
}

bool OSPathExists(str8 path)
{
    ArenaTemp temp = baseTempBegin(null, 0);

    path = Str8PushCopy(temp.arena, path);
    struct stat statBuf;
    bool found = stat((char *)path.data, &statBuf) == 0;
    
    baseTempEnd(temp);
    return found;
}

bool OSCreateDirectory(str8 path, bool createIntermediateDirs)
{
    bool result = true;
    ArenaTemp temp = baseTempBegin(null, 0);

    path = Str8PushCopy(temp.arena, path);
    
    if (createIntermediateDirs)
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

            if (mkdir((char *)path.data, S_IRWXU) != 0)
            {
                if (errno != EEXIST)
                {
                    result = false;
                    break;
                }
            }
        }

    }
    else
    {
        result = mkdir((char *)path.data, S_IRWXU);
    }

    baseTempEnd(temp);
    return result;
}

OSHandle OSFileOpen(str8 path, bool createLeadingDir, OSFileAccessFlags accessFlags, OSFileCreationKind creationKind)
{
    // handle create leading dir
    if (createLeadingDir)
    {
        str8 dirName = Str8ChopPastLastSlash(path);

        if(!OSPathExists(dirName))
        {
            OSCreateDirectory(dirName, true);
        }
    }

    int access = 0;
    if (accessFlags & OS_FILEACCESS_READ && accessFlags & OS_FILEACCESS_WRITE)
    {
        access |= O_RDWR;
    }
    else if (accessFlags & OS_FILEACCESS_READ)
    {
        access |= O_RDONLY;
    }
    else if (accessFlags & OS_FILEACCESS_WRITE)
    {
        access |= O_WRONLY;
    }

    switch(creationKind)
    {
        case OS_FILECREATION_CREATE_NEW: access |= O_CREAT; break;
        case OS_FILECREATION_CREATE_OVERRITE: access |= O_CREAT | O_TRUNC; break;
        case OS_FILECREATION_OPEN_ALWAYS: access |= O_CREAT; break;
        case OS_FILECREATION_OPEN_EXISTING: break; 
    }

    int fd = open((char *)path.data, access, S_IRWXU);
    return (OSHandle){._u64 = fd};
}

U8Array OSFileReadAll(struct Arena *arena, str8 path)
{
    int fd = open((char *)path.data, O_RDONLY);

    if (fd == -1)
    {
        return (U8Array){0};
    }

    u64 fileSize = OSGetFileSizeFromHandle((OSHandle){fd});
    u8 *data = arenaPushNoZero(arena, fileSize);
    read(fd, data, (u32)fileSize);
    close(fd);

    return (U8Array){.data = data, .len = fileSize};
}

void OSFileWrite(OSHandle fileHandle, u8 *bytes, u64 numBytes)
{
    int handle = (int)fileHandle._u64;

    write(handle, bytes, numBytes);
}

OSHandle OSGetCurrentThread(void)
{
    OSHandle handle = {._u64 = (u64)syscall(SYS_gettid)};

    return handle;
}
void OSSetThreadDebuggerName(OSHandle thread, str8 name)
{
    BASE_UNUSED_PARAM(thread);
    BASE_UNUSED_PARAM(name);
}

DateTime OSGetLocalTime(void)
{
    time_t now = time(null);
    struct tm local;

    localtime_r(&now, &local);

    return (DateTime)
    {
        .year = (u16)local.tm_year + 1900,
        .month = (u8)local.tm_mon,
        .dayOfWeek = (u8)local.tm_wday + 1,
        .day = (u8)local.tm_mday,
        .hour = (u8)local.tm_hour,
        .min = (u8)local.tm_min,
        .sec = (u8)local.tm_sec,
        .milli = (u16)0, //todo get milliseconds
    };
}
