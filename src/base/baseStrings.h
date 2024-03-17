#ifndef BASE_STRING_H
#define BASE_STRING_H

#include <stdarg.h>
#include "baseCoreTypes.h"

#define STR8_LIT(BYTES) baseStr8((BYTES), ((sizeof(BYTES)) - 1))
#define STR8_LIT_COMP(BYTES) (Str8){(BYTES), ((sizeof(BYTES)) - 1)}

typedef struct BaseStringBuilder
{
    u8* data;
    u64 cap;

    u64 len;

    //optional
    BaseArena *arena;
}BaseStringBuilder;

typedef struct Str8ListJoinParams
{
    str8 pre;
    str8 sep;
    str8 post;
}Str8ListJoinParams;

BASE_CREATE_LL_DECLS(Str8List, str8);
void Str8ListPushLastFmt(BaseArena *arena, Str8List *l, const u8 *fmt, ...);

str8 Str8ListJoin(BaseArena *arena, Str8List *l, Str8ListJoinParams *optionals);

// strings
str8 baseStr8(u8 *bytes, u64 size);
str16 baseStr16(u16 *bytes, u64 size);
str32 baseStr32(u32 *bytes, u64 size);

str8 baseStringsPushStr8Copy(BaseArena *arena, str8 str);
str8 baseStringsPushStr8FmtV(BaseArena *arena, const u8 *fmt, va_list args);
str8 baseStringsPushStr8Fmt(BaseArena *arena, const u8 *fmt, ...);

// string builder
BaseStringBuilder baseStringsSBCreate(BaseArena *arena, u64 cap);
void baseStringsSBAppendBytes(BaseStringBuilder *sb, const u8 *bytes, u64 count);
void baseStringsSBAppendCStr(BaseStringBuilder *sb, const u8 *str, i64 strSize);
void baseStringsSBAppendStr8(BaseStringBuilder *sb, str8 str);
void baseStringsSBAppendFmt(BaseStringBuilder *sb, const u8 *fmt, ...);

#endif