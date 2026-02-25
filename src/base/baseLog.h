#ifndef BASE_LOG_H
#define BASE_LOG_H

#include "base/baseMemory.h"
#include "base/baseStrings.h"
#include "base/baseThreads.h"
#include "os/core/osCore.h"

#define LOG_ENTRY_CHUNK_SIZE 50

typedef enum LogSeverityKind
{
    LOG_SEVERITY_ERROR,
    LOG_SEVERITY_WARNING,
    LOG_SEVERITY_INFO,
    LOG_SEVERITY_DEBUG,
}LogSeverityKind;

typedef struct LogEntry
{
    LogSeverityKind severity;
    DateTime time;
    str8 msg;
}LogEntry;

BASE_CREATE_ARRAY_VIEW_DECLS_DEFS(LogEntryArray, LogEntry)

typedef struct LogEntryChunkNode
{
    struct LogEntryChunkNode *next;
    struct LogEntryChunkNode *prev;
    LogEntryArray chunk;
    u64 cap;
}LogEntryChunkNode;

typedef struct LogEntryChunkList
{
    LogEntryChunkNode *first; 
    LogEntryChunkNode *last;

    //this is number of chunks
    u64 len;
    // this is number of messages in total
    u64 msgLen;
}LogEntryChunkList;

typedef struct Log
{
    LogEntryChunkList entries;
}Log;

str8 logFormatLogEntryMsg(Arena *arena, LogEntry msg);
void LogEntryChunkListPushNodeLast(LogEntryChunkList *l, LogEntryChunkNode *node);
void LogEntryChunkListPushNodeFirst(LogEntryChunkList *l, LogEntryChunkNode *node);
void LogEntryChunkListInsertNode(LogEntryChunkList *l, LogEntryChunkNode *prev, LogEntryChunkNode *node);
void LogEntryChunkListPush(Arena *arena, LogEntryChunkList *l, LogEntry msg);

str8 LogEntryChunkListJoin(Arena *arena, LogEntryChunkList *l);
LogEntryArray LogEntryChunkListFlattenToArray(Arena *arena, LogEntryChunkList *l);

Log *logCreate(Arena *arena);
void logClear(Log *log);

str8 logOutputToFile(Arena *arena, Log *log, str8 path);
void logOutputToConsole(Log *log);

void logPrintFmtV(Arena *arena, Log *log, LogSeverityKind severity, bool outputToConsole, char *fmt, va_list va);
void logPrintFmt(Arena *arena, Log *log, LogSeverityKind severity, char *fmt, ...);
void logErrorFmt(Arena *arena, Log *log, char *fmt, ...);
void logWarningFmt(Arena *arena, Log *log, char *fmt, ...);
void logInfoFmt(Arena *arena, Log *log, char *fmt, ...);
void logDebugFmt(Arena *arena, Log *log, char *fmt, ...);

str8 logThreadOutputToFile(void);
void logThreadOutputToConsole(void);

void logThreadPrintFmtV(LogSeverityKind severity, char *fmt, va_list va);
void logThreadPrintFmt(LogSeverityKind severity, char *fmt, ...);
void logThreadErrorFmt(char *fmt, ...);
void logThreadWarningFmt(char *fmt, ...);
void logThreadInfoFmt(char *fmt, ...);
void logThreadDebugFmt(char *fmt, ...);

#endif