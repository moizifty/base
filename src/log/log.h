#ifndef LOG_H
#define LOG_H

#include "base\baseMemory.h"
#include "base\baseStrings.h"
#include "base\baseThreads.h"
#include "os\core\osCore.h"

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

Log *logCreate(BaseArena *arena);
void logClose(Log *log);

void logClear(Log *log);

void logPrintFmtV(Log *log, LogSeverityKind severity, char *fmt, va_list va);
void logPrintFmt(Log *log, LogSeverityKind severity, char *fmt, ...);
void logErrorFmt(Log *log, char *fmt, ...);
void logWarningFmt(Log *log, char *fmt, ...);
void logInfoFmt(Log *log, char *fmt, ...);
void logDebugFmt(Log *log, char *fmt, ...);

void logProgPrintFmtV(LogSeverityKind severity, char *fmt, va_list va);
void logProgPrintFmt(LogSeverityKind severity, char *fmt, ...);
void logProgErrorFmt(char *fmt, ...);
void logProgWarningFmt(char *fmt, ...);
void logProgInfoFmt(char *fmt, ...);
void logProgDebugFmt(char *fmt, ...);

#endif