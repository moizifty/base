#include "baseLog.h"

u8 *gLogSeverityTable[] =
{
    [LOG_SEVERITY_ERROR] = (u8 *)"SeverityError",
    [LOG_SEVERITY_WARNING] = (u8 *)"SeverityWarning",
    [LOG_SEVERITY_INFO] = (u8 *)"SeverityInfo",
    [LOG_SEVERITY_DEBUG] = (u8 *)"SeverityDebug",
};

str8 logFormatLogEntryMsg(Arena *arena, LogEntry msg)
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
void LogEntryChunkListPush(Arena *arena, LogEntryChunkList *l, LogEntry msg)
{
    if(!BASE_ANY_PTR(l) || (l->last->chunk.len >= l->last->cap))
    {
        LogEntryChunkNode *n = arenaPush(arena, sizeof(LogEntryChunkNode));
        n->chunk.data = arenaPush(arena, sizeof(LogEntry) * LOG_ENTRY_CHUNK_SIZE);
        n->chunk.len = 0;
        n->cap = LOG_ENTRY_CHUNK_SIZE;

        LogEntryChunkListPushNodeLast(l, n);
    }
    
    l->last->chunk.data[l->last->chunk.len] = msg;
    l->last->chunk.len += 1;
    l->msgLen += 1;
}

str8 LogEntryChunkListJoin(Arena *arena, LogEntryChunkList *l)
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
LogEntryArray LogEntryChunkListFlattenToArray(Arena *arena, LogEntryChunkList *l)
{
    LogEntryArray flattened = {0};

    if (!BASE_ANY_PTR(l))
    {
        return flattened;
    }

    flattened.data = arenaPush(arena, sizeof(LogEntry) * l->msgLen);
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

Log *logCreate(Arena *arena)
{
    Log *l = arenaPush(arena, sizeof(Log));

    return l;
   
}
void logClear(Log *log)
{
    BASE_UNUSED_PARAM(log);
}

str8 logOutputToFile(Arena *arena, Log *log, str8 path)
{
    str8 logMsgAll = LogEntryChunkListJoin(arena, &log->entries);

    OSHandle logHandle = OSFileOpen(path, 
                              true, 
                              OS_FILEACCESS_READ | OS_FILEACCESS_WRITE,
                              OS_FILECREATION_CREATE_OVERRITE);

    OSFileWrite(logHandle, logMsgAll.data, logMsgAll.len);

    return logMsgAll;
}
void logOutputToConsole(Log *log)
{
    ArenaTemp temp =  baseTempBegin(null, 0);
    {
        BASE_LIST_FOREACH(LogEntryChunkNode, chunk, log->entries)
        {
            for(u64 i = 0; i < chunk->chunk.len; i++)
            {
                LogEntry entry = chunk->chunk.data[i];
                str8 msgFmt = logFormatLogEntryMsg(temp.arena, entry);
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
}

void logPrintFmtV(Arena *arena, Log *log, LogSeverityKind severity, bool outputToConsole, char *fmt, va_list va)
{
    str8 s = Str8PushFmtV(arena, fmt, va);
    LogEntry entry = {.msg = s, .severity = severity, .time = OSGetLocalTime()};
    LogEntryChunkListPush(arena, &log->entries, entry);

    if (outputToConsole)
    {
        if (severity == LOG_SEVERITY_ERROR) baseColEPrintf("{r}%S\n", s);
        else baseColPrintf("%S\n", s);
    }
}
void logPrintFmt(Arena *arena, Log *log, LogSeverityKind severity, char *fmt, ...)
{
    va_list list;
    va_start(list, fmt);

    logPrintFmtV(arena, log, severity, false, fmt, list);

    va_end(list);
}
void logErrorFmt(Arena *arena, Log *log, char *fmt, ...)
{
    va_list list;
    va_start(list, fmt);

    logPrintFmtV(arena, log, LOG_SEVERITY_ERROR, false, fmt, list);

    va_end(list);
}
void logWarningFmt(Arena *arena, Log *log, char *fmt, ...)
{
    va_list list;
    va_start(list, fmt);

    logPrintFmtV(arena, log, LOG_SEVERITY_WARNING, false, fmt, list);

    va_end(list);
}
void logInfoFmt(Arena *arena, Log *log, char *fmt, ...)
{
    va_list list;
    va_start(list, fmt);

    logPrintFmtV(arena, log, LOG_SEVERITY_INFO, false, fmt, list);

    va_end(list);
}
void logDebugFmt(Arena *arena, Log *log, char *fmt, ...)
{
    va_list list;
    va_start(list, fmt);

    logPrintFmtV(arena, log, LOG_SEVERITY_DEBUG, false, fmt, list);

    va_end(list);
}

str8 logThreadOutputToFile(void)
{
    BaseThreadCtx *threadCtx = baseThreadsGetCtx();

    str8 logDir = OSGetState()->thisProcState.logDirPath;
    str8 logPath = Str8PushFmt(threadCtx->threadLogArena, "%S\\[%S]_log.txt", logDir, baseThreadsGetName());

    return logOutputToFile(threadCtx->threadLogArena, threadCtx->threadLog, logPath);
}
void logThreadOutputToConsole(void)
{
    BaseThreadCtx *threadCtx = baseThreadsGetCtx();

    logOutputToConsole(threadCtx->threadLog);
}

void logThreadPrintFmtV(LogSeverityKind severity, char *fmt, va_list va)
{
    BaseThreadCtx *threadCtx = baseThreadsGetCtx();

#if defined(BASE_LOG_OUTPUT_TO_CONSOLE)
    bool outputToConsole = true;
#else
    bool outputToConsole = false;
#endif

    logPrintFmtV(threadCtx->threadLogArena, threadCtx->threadLog, severity, outputToConsole, fmt, va);
}
void logThreadPrintFmt(LogSeverityKind severity, char *fmt, ...)
{
    BaseThreadCtx *threadCtx = baseThreadsGetCtx();

    va_list list;
    va_start(list, fmt);

#ifdef BASE_LOG_OUTPUT_TO_CONSOLE
    bool outputToConsole = true;
#else
    bool outputToConsole = false;
#endif

    logPrintFmtV(threadCtx->threadLogArena, threadCtx->threadLog, severity, outputToConsole, fmt, list);

    va_end(list);
}
void logThreadErrorFmt(char *fmt, ...)
{
    BaseThreadCtx *threadCtx = baseThreadsGetCtx();

    va_list list;
    va_start(list, fmt);

#ifdef BASE_LOG_OUTPUT_TO_CONSOLE
    bool outputToConsole = true;
#else
    bool outputToConsole = false;
#endif

    logPrintFmtV(threadCtx->threadLogArena, threadCtx->threadLog, LOG_SEVERITY_ERROR, outputToConsole, fmt, list);

    va_end(list);
}
void logThreadWarningFmt(char *fmt, ...)
{
    BaseThreadCtx *threadCtx = baseThreadsGetCtx();

    va_list list;
    va_start(list, fmt);

#ifdef BASE_LOG_OUTPUT_TO_CONSOLE
    bool outputToConsole = true;
#else
    bool outputToConsole = false;
#endif

    logPrintFmtV(threadCtx->threadLogArena, threadCtx->threadLog, LOG_SEVERITY_WARNING, outputToConsole, fmt, list);

    va_end(list);
}
void logThreadInfoFmt(char *fmt, ...)
{
    BaseThreadCtx *threadCtx = baseThreadsGetCtx();

    va_list list;
    va_start(list, fmt);

#ifdef BASE_LOG_OUTPUT_TO_CONSOLE
    bool outputToConsole = true;
#else
    bool outputToConsole = false;
#endif

    logPrintFmtV(threadCtx->threadLogArena, threadCtx->threadLog, LOG_SEVERITY_INFO, outputToConsole, fmt, list);

    va_end(list);
}
void logThreadDebugFmt(char *fmt, ...)
{
    BaseThreadCtx *threadCtx = baseThreadsGetCtx();

    va_list list;
    va_start(list, fmt);

#ifdef BASE_LOG_OUTPUT_TO_CONSOLE
    bool outputToConsole = true;
#else
    bool outputToConsole = false;
#endif

    logPrintFmtV(threadCtx->threadLogArena, threadCtx->threadLog, LOG_SEVERITY_DEBUG, outputToConsole, fmt, list);

    va_end(list);
}