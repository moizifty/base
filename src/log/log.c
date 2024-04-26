#include "log.h"

str8 logGetLogsDirectory(BaseArena *arena)
{
    str8 dir = OSGetProgramDirectoryPath(arena);

    Str8List strs = {0};
    Str8ListPushLastFmt(arena, &strs, "%S\\%S\\",dir, LOG_FOLDER_NAME);

    return Str8ListJoin(arena, &strs, null);
}