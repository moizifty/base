#ifndef OS_CORE_H
#define OS_CORE_H

#include "base\baseCoreTypes.h"
#include "base\baseMath.h"

#define LOG_FOLDER_NAME STR8_LIT("logs")

typedef struct OSProcessState
{
    str8 binaryPath;
    str8 logDirPath;

    struct Log *processLog;
}OSProcessState;

typedef struct OSStatePlatform
{
    u64 _opaque;
}OSStatePlatform;

typedef struct OSState
{
    OSProcessState thisProcState;

    OSStatePlatform *platformSpecific;
}OSState;

typedef struct OSHandle
{
    u64 _u64;
}OSHandle;

typedef enum OSFileAccessFlags
{
    OS_FILEACCESS_READ = 1 << 0,
    OS_FILEACCESS_WRITE = 1 << 1,
}OSFileAccessFlags;

typedef enum OSFileCreationKind
{
    OS_FILECREATION_CREATE_NEW,
    OOS_FILECREATION_CREATE_OVERRITE,
    OS_FILECREATION_OPEN_EXISTING,
    OS_FILECREATION_OPEN_ALWAYS,
}OSFileCreationKind;

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

typedef struct OSExceptionInfo
{
    //todo put meaningfull platform independent info heres
    u32 _placeholder;
}OSExceptionInfo;

#ifdef OS_WINDOWS
#include "win32\osCoreWin32.h"
#else
#error Platform not defined
#endif

OSState *OSInit(BaseArena *arena);
OSState *OSGetState(void);

void* OSReserveMemory(u64 size);
void OSCommitMemory(void *ptr, u64 size);
void OSDecommitMemory(void *ptr, u64 size);
void OSFreeMemory(void *ptr, u64 size);

void OSEnableVirtualTerminalSequenceProcessing(void);

// files
OSHandle OSFileOpen(str8 path, bool createLeadingDir, OSFileAccessFlags accessFlags, OSFileCreationKind creationKind);
void OSFileWrite(OSHandle fileHandle, u8 *bytes, u64 numBytes);
void OSFileWriteStr8(OSHandle fileHandle, str8 str);
void OSFileWriteFmt(OSHandle fileHandle, char *fmt, ...);
void OSFileClose(OSHandle handle);

bool OSPathExists(str8 path);
bool OSPathIsDirectory(str8 path);
u64 OSGetFileSize(str8 path);
u64 OSGetFileSizeFromHandle(OSHandle handle);
u8 *OSReadFileAll(struct BaseArena *arena, str8 path, u64 *outFileSize);
str8 OSGetFullPath(struct BaseArena *arena, str8 path);

bool OSCreateDirectory(str8 path, bool createIntermediateDirs);

OSFileFindIter *OSFindFileBegin(struct BaseArena *arena, str8 path, OSFileFindOptionalParams *opt);
bool OSFindFileNext(struct BaseArena *arena, OSFileFindIter *iter, OSFileInfo *out);
void OSFindFileEnd(OSFileFindIter *iter);

// process
str8 OSGetProgramPath(BaseArena *arena);
str8 OSGetProgramDirectory(BaseArena *arena);
str8 OSGetProgramLogsDirectory(BaseArena *arena);
bool OSRunProcessEx(struct BaseArena *arena, str8 app, str8 args, void *peb, str8 *outStr, str8 *errStr);

// Date and time
DateTime OSGetSytemTime(void);
DateTime OSGetLocalTime(void);

// other
range2i OSClientRectFromWindow(OSHandle handle);

global OSState *gOSState;
#endif