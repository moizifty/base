#ifndef BASE_PATH_H
#define BASE_PATH_H

#include "base/baseCore.h"
#include "base/baseStrings.h"
#include "base/baseMemory.h"
#include "base/baseThreads.h"

typedef struct Path
{
    str8 path;
    Str8List directories;
    str8 filename;
    str8 extention;
}Path;

Path pathFromStr8(Arena *arena, str8 path);

#endif