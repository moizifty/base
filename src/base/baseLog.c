#include "baseLog.h"

u8 *gLogSeverityTable[] =
{
    [LOG_SEVERITY_ERROR] = (u8 *)"SeverityError",
    [LOG_SEVERITY_WARNING] = (u8 *)"SeverityWarning",
    [LOG_SEVERITY_INFO] = (u8 *)"SeverityInfo",
    [LOG_SEVERITY_DEBUG] = (u8 *)"SeverityDebug",
};

str8 logFormatLogEntryMsg(BaseArena *arena, LogEntry msg)
{
    DateTime time = msg.time;
    Str8List list = {0};

    Str8ListPushLastFmt(arena, 
                        &list,
                        "[%02d/%02d/%04d <-> %02d:%02d:%2d][%s]: ", 
                        time.day, 
                        time.month, 
                        time.year,
                        time.hour,
                        time.min,
                        time.sec,
                        gLogSeverityTable[msg.severity]);

    Str8ListPushLast(arena, &list, msg.msg);

    return Str8ListJoin(arena, &list, null);
}

void LogEntryChunkListPushNodeLast(LogEntryChunkList *l, LogEntryChunkNode *node)
{
    BaseDllNodePushLast(l->first, l->last, node);
	l->len += 1;
	l->msgLen += node->chunk.len;
}
void LogEntryChunkListPushNodeFirst(LogEntryChunkList *l, LogEntryChunkNode *node)
{
    BaseDllNodePushFirst(l->first, l->last, node);
	l->len += 1;
	l->msgLen += node->chunk.len;
}
void LogEntryChunkListInsertNode(LogEntryChunkList *l, LogEntryChunkNode *prev, LogEntryChunkNode *node)
{
    BaseDllNodeInsert(l->first, l->last, prev, node);
	l->len += 1;
	l->msgLen += node->chunk.len;
}
void LogEntryChunkListPush(BaseArena *arena, LogEntryChunkList *l, LogEntry msg)
{
    if(!BASE_ANY_PTR(l) || (l->last->chunk.len >= l->last->cap))
    {
        LogEntryChunkNode *n = baseArenaPush(arena, sizeof(LogEntryChunkNode));
        n->chunk.data = baseArenaPush(arena, sizeof(LogEntry) * LOG_ENTRY_CHUNK_SIZE);
        n->chunk.len = 0;
        n->cap = LOG_ENTRY_CHUNK_SIZE;

        LogEntryChunkListPushNodeLast(l, n);
    }
    
    l->last->chunk.data[l->last->chunk.len] = msg;
    l->last->chunk.len += 1;
    l->msgLen += 1;
}

str8 LogEntryChunkListJoin(BaseArena *arena, LogEntryChunkList *l)
{
    LogEntryArray flattened = LogEntryChunkListFlattenToArray(arena, l);

    if(flattened.len == 0)
    {
        return STR8_LIT("");
    }

    Str8List list = {0};
    for(u64 i = 0; i < flattened.len; i++)
    {
        str8 formatted = logFormatLogEntryMsg(arena, flattened.data[i]);

        Str8ListPushLast(arena, &list, formatted);
    }

    return Str8ListJoin(arena, &list, &(Str8ListJoinParams){.sep = STR8_LIT("\n")});
}
LogEntryArray LogEntryChunkListFlattenToArray(BaseArena *arena, LogEntryChunkList *l)
{
    LogEntryArray flattened = {0};

    if (!BASE_ANY_PTR(l))
    {
        return flattened;
    }

    flattened.data = baseArenaPush(arena, sizeof(LogEntry) * l->msgLen);
    flattened.len = l->msgLen;

    u64 i = 0;
    BASE_PTR_LIST_FOREACH(LogEntryChunkNode, chunk, l)
    {
        for(u64 j = 0; j < chunk->chunk.len; j++)
        {
            flattened.data[i++] = chunk->chunk.data[j];
        }
    }

    return flattened;
}

CRITICAL_SECTION sec;
Log *logCreate(BaseArena *arena)
{
    Log *l = baseArenaPush(arena, sizeof(Log));

    BaseArena *logArena = OSGetState()->thisProcState.logArena;
    str8 logDir = OSGetState()->thisProcState.logDirPath;
    str8 logPath = STR8_LIT("");
    Str8List logPathList = {0};

    Str8ListPushLastFmt(logArena, &logPathList, "%S\\log.txt", logDir);

    logPath = Str8ListJoin(logArena, &logPathList, null);

    l->logHandle = OSFileOpen(logPath, 
                                true, 
                                OS_FILEACCESS_READ | OS_FILEACCESS_WRITE,
                                OOS_FILECREATION_CREATE_OVERRITE);

    return l;
   
}
str8 logFlush(Log *log)
{
    BaseArena *logArena = OSGetState()->thisProcState.logArena;
    str8 logMsgAll = LogEntryChunkListJoin(logArena, &log->entries);
    OSFileWrite(log->logHandle, logMsgAll.data, logMsgAll.len);

    return logMsgAll;
}
void logClose(Log *log)
{
    logFlush(log);
    OSFileClose(log->logHandle);
}
void logClear(Log *log)
{
    BASE_UNUSED_PARAM(log);
}

void logOutputToConsole(Log *log)
{
    BaseArena *logArena = OSGetState()->thisProcState.logArena;
    BASE_LIST_FOREACH(LogEntryChunkNode, chunk, log->entries)
    {
        for(u64 i = 0; i < chunk->chunk.len; i++)
        {
            LogEntry entry = chunk->chunk.data[i];
            str8 msgFmt = logFormatLogEntryMsg(logArena, entry);
            switch(entry.severity)
            {
                case LOG_SEVERITY_WARNING:
                {
                    baseColPrintf("{o}%S\n", msgFmt);
                }break;

                case LOG_SEVERITY_DEBUG:
                {
                    baseColPrintf("{g}%S\n", msgFmt);
                }break;

                case LOG_SEVERITY_INFO:
                {
                    baseColPrintf("{b}%S\n", msgFmt);
                }break;

                case LOG_SEVERITY_ERROR:
                {
                    baseColPrintf("{r}%S\n", msgFmt);
                }break;
            }
        }
    }
}

void logPrintFmtV(Log *log, LogSeverityKind severity, char *fmt, va_list va)
{
    BaseArena *logArena = OSGetState()->thisProcState.logArena;

    str8 s = baseStringsPushStr8FmtV(logArena, fmt, va);
    LogEntry entry = {.msg = s, .severity = severity, .time = OSGetLocalTime()};
    LogEntryChunkListPush(logArena, &log->entries, entry);
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