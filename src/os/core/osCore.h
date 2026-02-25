#ifndef OS_CORE_H
#define OS_CORE_H

#include "base/baseCoreTypes.h"
#include "base/baseMath.h"

#ifndef arenaReserveImpl
#define arenaReserveImpl OSReserveMemory
#endif
#ifndef arenaCommitImpl
#define arenaCommitImpl OSCommitMemory
#endif
#ifndef arenaDecommitImpl
#define arenaDecommitImpl OSDecommitMemory
#endif
#ifndef arenaFreeImpl
#define arenaFreeImpl OSFreeMemory
#endif

#define LOG_FOLDER_NAME STR8_LIT("logs")

typedef struct OSProcessState
{
    str8 binaryPath;
    str8 logDirPath;

    // Arena *logArena;
    // struct Log *processLog;
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
    struct OSHandle *next;
    struct OSHandle *prev;
}OSHandle;

typedef enum OSFileAccessFlags
{
    OS_FILEACCESS_READ = 1 << 0,
    OS_FILEACCESS_WRITE = 1 << 1,
}OSFileAccessFlags;

typedef enum OSFileCreationKind
{
    OS_FILECREATION_CREATE_NEW,
    OS_FILECREATION_CREATE_OVERRITE,
    OS_FILECREATION_OPEN_EXISTING,
    OS_FILECREATION_OPEN_ALWAYS,
}OSFileCreationKind;

typedef enum OSFileFindType
{
    OS_FILEFIND_NONE,
}OSFileFindType;

typedef u64 OSFileAttributeFlags;
enum
{
    OS_FILEATTR_DIR = (1<<0),
};

typedef enum OSKey
{
    OS_KEY_NULL,
    
    OS_KEY_A,
    OS_KEY_B,
    OS_KEY_C,
    OS_KEY_D,
    OS_KEY_E,
    OS_KEY_F,
    OS_KEY_G,
    OS_KEY_H,
    OS_KEY_I,
    OS_KEY_J,
    OS_KEY_K,
    OS_KEY_L,
    OS_KEY_M,
    OS_KEY_N,
    OS_KEY_O,
    OS_KEY_P,
    OS_KEY_Q,
    OS_KEY_R,
    OS_KEY_S,
    OS_KEY_T,
    OS_KEY_U,
    OS_KEY_V,
    OS_KEY_W,
    OS_KEY_X,
    OS_KEY_Y,
    OS_KEY_Z,
    OS_KEY_COUNT,
}OSKey;

typedef struct OSKeyState
{
    bool pressed;
}OSKeyState;

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
    str8 name;
    OSFileAttributeFlags attrs;
}OSFileInfo;

typedef struct OSExceptionInfo
{
    //todo put meaningfull platform independent info heres
    u32 _placeholder;
}OSExceptionInfo;

#if OS_WIN32 == 1
#include "win32/osCoreWin32.h"
#elif OS_LINUX == 1
#include "linux/osCoreLinux.h"
#else
#error Platform not defined
#endif

OSState *OSInit(Arena *arena);
OSState *OSGetState(void);

bool OSHandleEquals(OSHandle a, OSHandle b);
bool OSIsHandleValid(OSHandle handle);
void* OSReserveMemory(u64 size);
void OSCommitMemory(void *ptr, u64 size);
void OSDecommitMemory(void *ptr, u64 size);
void OSFreeMemory(void *ptr, u64 size);

void OSEnableVirtualTerminalSequenceProcessing(void);

// files
OSHandle OSFileOpen(str8 path, bool createLeadingDir, OSFileAccessFlags accessFlags, OSFileCreationKind creationKind);
void OSFileWrite(OSHandle fileHandle, u8 *bytes, u64 numBytes);
void OSFileWriteStr8(OSHandle fileHandle, str8 str);
void OSFileWriteU32(OSHandle fileHandle, u32 n);
void OSFileWriteU64(OSHandle fileHandle, u64 n);
void OSFileWriteFmt(OSHandle fileHandle, char *fmt, ...);

U8Array OSFileReadAll(struct Arena *arena, str8 path);
bool OSFileWriteAll(str8 path, U8Array bytes, bool createLeadingDir, bool overwrite);
bool OSFileWriteAllStr8(str8 path, str8 str, bool createLeadingDir, bool overwrite);

void OSFileClose(OSHandle handle);

bool OSPathExists(str8 path);
bool OSPathIsDirectory(str8 path);
u64 OSGetFileSize(str8 path);
u64 OSGetFileSizeFromHandle(OSHandle handle);
str8 OSGetFullPath(struct Arena *arena, str8 path);

bool OSCreateDirectory(str8 path, bool createIntermediateDirs);

OSFileFindIter *OSFindFileBegin(struct Arena *arena, str8 path, OSFileFindOptionalParams *opt);
bool OSFindFileNext(struct Arena *arena, OSFileFindIter *iter, OSFileInfo *out);
void OSFindFileEnd(OSFileFindIter *iter);

Str8List OSGetFilePaths(Arena *arena, str8 dir, str8 pattern, bool recursive);

// process
str8 OSGetProgramPath(Arena *arena);
str8 OSGetProgramDirectory(Arena *arena);
str8 OSGetProgramLogsDirectory(Arena *arena);
bool OSRunProcessEx(struct Arena *arena, str8 app, str8 args, void *peb, str8 *outStr, str8 *errStr);

OSHandle OSLoadDynamicLibrary(str8 name);
void *OSGetExportAddressFromDynamicLibrary(OSHandle dynLib, str8 name);

// Date and time
DateTime OSGetSytemTime(void);
DateTime OSGetLocalTime(void);

u64 OSGetPerformanceCounter(void);
u64 OSGetPerformanceFrequency(void);

f64 OSConvertPerformanceCounterToMillisecondsF64(u64 counter);
//env
str8 OSGetEnvironmentVar(Arena *arena, str8 var);

// other
vec2i OSScreenCoordToClientCoord(OSHandle wndHandle, vec2i screen);
range2i OSClientRectFromWindow(OSHandle handle);
vec2i OSGetCursorScreenCoordPos(void);
vec2i OSGetCursorClientCoordPos(OSHandle wndHandle);

//threading
OSHandle OSGetCurrentThread(void);
void OSSetThreadDebuggerName(OSHandle thread, str8 name);
str8 OSGetThreadDebuggerName(OSHandle thread);

BASE_CREATE_EFFICIENT_LL_DECLS(OSHandleList, OSHandle)

global u64 gOSPerformanceFreq;
global OSState *gOSState;

#endif