#ifndef BASE_STRING_H
#define BASE_STRING_H

#include <stdarg.h>
#include "baseCoreTypes.h"

typedef struct BaseStringBuilder
{
    u8* data;
    u64 cap;

    u64 len;

    //optional
    BaseArena *arena;
}BaseStringBuilder;

BaseStringBuilder baseStringsSBCreate(BaseArena *arena, u64 cap);
void baseStringsSBAppendCStr(BaseStringBuilder *sb, const u8 *str, i64 strSize);
void baseStringsSBAppendFmt(BaseStringBuilder *sb, const u8 *fmt, ...);

#endif