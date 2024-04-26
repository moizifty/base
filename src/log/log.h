#ifndef LOG_H
#define LOG_H

#include "base\baseMemory.h"
#include "base\baseStrings.h"
#include "base\baseThreads.h"
#include "os\core\osCore.h"

#define LOG_FOLDER_NAME STR8_LIT("logs")
typedef struct Log
{
    OSHandle logHandle;
}Log;

typedef enum LogSeverityKind
{
    LOG_SEVERITY_ERROR,
    LOG_SEVERITY_WARNING,
    LOG_SEVERITY_INFO,
    LOG_SEVERITY_DEBUG,
}LogSeverityKind;

str8 logGetLogsDirectory(BaseArena *arena);
Log *logCreate(BaseArena *arena);
void logClose(Log *log);

void logClear(Log *log);

void logPrintFmtV(Log *log, LogSeverityKind severity, char *fmt, va_list va);
void logPrintFmt(Log *log, LogSeverityKind severity, char *fmt, ...);
void logErrorFmt(Log *log, char *fmt, ...);
void logWarningFmt(Log *log, char *fmt, ...);
void logInfoFmt(Log *log, char *fmt, ...);
void logDebugFmt(Log *log, char *fmt, ...);
#endif