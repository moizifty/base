#include "log.h"

u8 *gLogSeverityTable[] =
{
    [LOG_SEVERITY_ERROR] = "SeverityError",
    [LOG_SEVERITY_WARNING] = "SeverityWarning",
    [LOG_SEVERITY_INFO] = "SeverityInfo",
    [LOG_SEVERITY_DEBUG] = "SeverityDebug",
};

Log *logCreate(BaseArena *arena)
{
    Log *l = baseArenaPush(arena, sizeof(Log));

    BaseArenaTemp temp = baseTempBegin(&arena, 1);
    {
        str8 logDir = OSGetState()->thisProcState.logDirPath;
        str8 logPath = STR8_LIT("");
        Str8List logPathList = {0};

        Str8ListPushLastFmt(temp.arena, &logPathList, "%S\\log.txt", logDir);

        logPath = Str8ListJoin(temp.arena, &logPathList, null);

        l->logHandle = OSFileOpen(logPath, 
                                  true, 
                                  OS_FILEACCESS_READ | OS_FILEACCESS_WRITE,
                                  OOS_FILECREATION_CREATE_OVERRITE);
    }
    baseTempEnd(temp);

    return l;
   
}
void logClose(Log *log)
{
    OSFileClose(log->logHandle);
}

void logClear(Log *log)
{

}

void logPrintFmtV(Log *log, LogSeverityKind severity, char *fmt, va_list va)
{
    BaseArenaTemp temp = baseTempBegin(null, 0);
    {
        DateTime time = OSGetLocalTime();
        str8 preStr = baseStringsPushStr8Fmt(temp.arena, 
                                             "[%02d/%02d/%04d <-> %02d:%02d:%2d][%s]: ", 
                                             time.day, 
                                             time.month, 
                                             time.year,
                                             time.hour,
                                             time.min,
                                             time.sec,
                                             gLogSeverityTable[severity]);

        str8 s = baseStringsPushStr8FmtV(temp.arena, fmt, va);

        OSFileWrite(log->logHandle, preStr.data, preStr.len);
        OSFileWrite(log->logHandle, s.data, s.len);

        u8 nl = '\n';
        OSFileWrite(log->logHandle, &nl, 1);
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

void logProgPrintFmtV(LogSeverityKind severity, char *fmt, va_list va)
{
    logPrintFmtV(OSGetState()->thisProcState.processLog, severity, fmt, va);
}
void logProgPrintFmt(LogSeverityKind severity, char *fmt, ...)
{
    va_list list;
    va_start(list, fmt);

    logPrintFmtV(OSGetState()->thisProcState.processLog, severity, fmt, list);

    va_end(list);
}
void logProgErrorFmt(char *fmt, ...)
{
    va_list list;
    va_start(list, fmt);

    logPrintFmtV(OSGetState()->thisProcState.processLog, LOG_SEVERITY_ERROR, fmt, list);

    va_end(list);
}
void logProgWarningFmt(char *fmt, ...)
{
    va_list list;
    va_start(list, fmt);

    logPrintFmtV(OSGetState()->thisProcState.processLog, LOG_SEVERITY_WARNING, fmt, list);

    va_end(list);
}
void logProgInfoFmt(char *fmt, ...)
{
    va_list list;
    va_start(list, fmt);

    logPrintFmtV(OSGetState()->thisProcState.processLog, LOG_SEVERITY_INFO, fmt, list);

    va_end(list);
}
void logProgDebugFmt(char *fmt, ...)
{
    va_list list;
    va_start(list, fmt);

    logPrintFmtV(OSGetState()->thisProcState.processLog, LOG_SEVERITY_DEBUG, fmt, list);

    va_end(list);
}