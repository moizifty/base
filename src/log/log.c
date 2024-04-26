#include "log.h"

str8 logGetLogsDirectory(BaseArena *arena)
{
    str8 dir = OSGetProgramDirectoryPath(arena);

    Str8List strs = {0};
    Str8ListPushLastFmt(arena, &strs, "%S\\%S\\",dir, LOG_FOLDER_NAME);

    return Str8ListJoin(arena, &strs, null);
}

Log *logCreate(BaseArena *arena)
{

}
void logClose(Log *log)
{

}

void logClear(Log *log)
{

}

void logPrintFmtV(Log *log, LogSeverityKind severity, char *fmt, va_list va)
{
    BaseArenaTemp temp = baseTempBegin(null, 0);
    {
        str8 s = baseStringsPushStr8FmtV(temp.arena, fmt, va);
    }
    baseTempEnd(temp);
}
void logPrintFmt(Log *log, LogSeverityKind severity, char *fmt, ...)
{
    va_list list;
    va_start(list, fmt);

    logPrintFmtV(log, severity, fmt, list);

    va_end(list);
}
void logErrorFmt(Log *log, char *fmt, ...)
{
    va_list list;
    va_start(list, fmt);

    logPrintFmtV(log, LOG_SEVERITY_ERROR, fmt, list);

    va_end(list);
}
void logWarningFmt(Log *log, char *fmt, ...)
{
    va_list list;
    va_start(list, fmt);

    logPrintFmtV(log, LOG_SEVERITY_WARNING, fmt, list);

    va_end(list);
}
void logInfoFmt(Log *log, char *fmt, ...)
{
    va_list list;
    va_start(list, fmt);

    logPrintFmtV(log, LOG_SEVERITY_INFO, fmt, list);

    va_end(list);
}
void logDebugFmt(Log *log, char *fmt, ...)
{
    va_list list;
    va_start(list, fmt);

    logPrintFmtV(log, LOG_SEVERITY_DEBUG, fmt, list);

    va_end(list);
}