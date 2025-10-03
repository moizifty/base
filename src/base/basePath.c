#include "basePath.h"

Path pathFromStr8(Arena *arena, str8 path)
{
    Path p = {.path = path};

    if (!Str8IsNullOrEmpty(path))
    {
        p.directories = Str8Split(arena, path, STR8_LIT("\\"), STR_MATCHFLAGS_SLASH_INSENSITIVE, STR_SPLITFLAGS_NO_COPY| STR_SPLITFLAGS_DISCARD_EMPTY);
        p.filename = p.directories.last->val;
        if (Str8Contains(p.filename, STR8_LIT("."), 0))
        {
            p.extention = Str8ChopBefore(p.filename, STR8_LIT("."), STR_MATCHFLAGS_FIND_LAST);
            p.filename = Str8ChopPast(p.filename, STR8_LIT("."), STR_MATCHFLAGS_FIND_LAST);
        }

        if (BASE_ANY(p.directories))
        {
            Str8ListPopNodeLast(&p.directories);
        }
    }

    return p;
}