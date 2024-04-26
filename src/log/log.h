#ifndef LOG_H
#define LOG_H

#include "base\baseMemory.h"
#include "base\baseStrings.h"
#include "os\core\osCore.h"

#define LOG_FOLDER_NAME STR8_LIT("logs")
typedef struct Log
{
    OSHandle logHandle;
}Log;

str8 logGetLogsDirectory(BaseArena *arena);
Log *logCreate(BaseArena *arena);

#endif