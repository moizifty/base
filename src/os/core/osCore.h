#ifndef OS_CORE_H
#define OS_CORE_H

#include "base\baseCoreTypes.h"

typedef struct OSHandle
{
    u64 _u64;
}OSHandle;

typedef enum OSFileFindType
{
    OS_FILEFIND_NONE,
    OS_FILEFIND_TYPE_TOP_LEVEL_DIR,
}OSFileFindType;

typedef u64 OSFileAttributeFlags;
enum
{
    OS_FILEATTR_DIR = (1<<0),
};

typedef struct OSFileFindOptionalParams
{
    OSFileFindType type;
}OSFileFindOptionalParams;

typedef struct OSFileFindIter
{
    u8 _opaque;
}OSFileFindIter;

typedef struct OSFileInfo
{
    str8 path;
    OSFileAttributeFlags attrs;
}OSFileInfo;

#ifdef OS_WINDOWS
#include "win32\osCoreWin32.h"
#else
#error Platform not defined
#endif

void* OSReserveMemory(u64 size);
void OSCommitMemory(void *ptr, u64 size);
void OSDecommitMemory(void *ptr, u64 size);
void OSFreeMemory(void *ptr, u64 size);

void OSEnableVirtualTerminalSequenceProcessing(void);

// files
bool OSPathIsDirectory(str8 path);
u64 OSGetFileSize(str8 path);
u64 OSGetFileSizeFromHandle(OSHandle handle);
u8 *OSReadFileAll(struct BaseArena *arena, str8 path, u64 *outFileSize);

OSFileFindIter *OSFindFileBegin(struct BaseArena *arena, str8 path, OSFileFindOptionalParams *opt);
bool OSFindFileNext(struct BaseArena *arena, OSFileFindIter *iter, OSFileInfo *out);

// process
str8 OSGetProgramPath(BaseArena *arena);
str8 OSGetProgramDirectoryPath(BaseArena *arena);
bool OSRunProcessEx(struct BaseArena *arena, str8 app, str8 args, void *peb, str8 *outStr, str8 *errStr);
#endif