#include "baseMemory.h"
#include "baseStrings.h"
#include "baseThreads.h"

BASE_CREATE_LL_DEFS_EX(Str8List, Str8ListNode, str8, sizeof(node->val.len));

void Str8ListPushLastFmt(BaseArena *arena, Str8List *l, const u8 *fmt, ...)
{
    va_list list;
    va_start(list, fmt);

    str8 s = baseStringsPushStr8FmtV(arena, fmt, list);

    va_end(list);

    Str8ListPushLast(arena, l, s);
}

str8 Str8ListJoin(BaseArena *arena, Str8List *l, Str8ListJoinParams *optionals)
{
    Str8ListJoinParams params = {0};
    if (optionals != null)
    {
        params = *optionals;
    }

    str8 result = {0};
}

str8 baseStr8(u8 *bytes, u64 size)
{
    return (str8) {.data = bytes, .len = size};
}
str16 baseStr16(u16 *bytes, u64 size)
{
    return (str16) {.data = bytes, .len = size};
}
str32 baseStr32(u32 *bytes, u64 size)
{
    return (str32) {.data = bytes, .len = size};
}

str8 baseStringsPushStr8Copy(BaseArena *arena, str8 str)
{
    str8 s = {0};
    s.data = baseArenaPushNoZero(arena, str.len + 1);
    s.len = str.len;

    BASE_MEMCPY(s.data, str.data, s.len);
    s.data[s.len] = '\0';

    return s;
}
str8 baseStringsPushStr8FmtV(BaseArena *arena, const u8 *fmt, va_list args)
{
    va_list list;
    va_copy(list, args);

    char tempBuf[1];
    i64 numWritten = vsnprintf(tempBuf, 1, fmt, list);

    str8 s = {0};
    s.data = baseArenaPushNoZero(arena, numWritten + 1);
    s.len = numWritten;

    vsnprintf(s.data, s.len + 1, fmt, list);
    return s;
}
str8 baseStringsPushStr8Fmt(BaseArena *arena, const u8* fmt, ...)
{
    va_list list;
    va_start(list, fmt);

    str8 s = baseStringsPushStr8FmtV(arena, fmt, list);

    va_end(list);
    return s;
}

BaseStringBuilder baseStringsCreateSB(BaseArena *arena, u64 cap)
{
    BaseStringBuilder builder = {0};
    
    // dont allocate from this arena but
    // have specialist functions that do that
    builder.arena = arena;
    builder.cap = cap;
    builder.len = 0;
    builder.data = baseArenaPush(arena, cap);

    return builder;
}
void baseStringsSBAppendBytes(BaseStringBuilder *sb, const u8 *bytes, u64 count)
{
    if((sb->len + count) > sb->cap)
    {
        baseArenaPush(sb->arena, count);
        sb->cap += count;
    }

    BASE_MEMCPY(sb->data + sb->len, bytes, count);
    sb->len += count;
}
void baseStringsSBAppendCStr(BaseStringBuilder *sb, const u8 *str, i64 strSize)
{
    u64 strLength = (strSize == -1) ? strlen(str) : strSize;
    baseStringsSBAppendBytes(sb, str, strLength + 1); //+ 1 for coppying '\0'
    sb->len -= 1; //since we coppied the '\0' over
}
void baseStringsSBAppendStr8(BaseStringBuilder *sb, str8 str)
{
    baseStringsSBAppendBytes(sb, str.data, str.len);

    u8 ch = '\0';
    baseStringsSBAppendBytes(sb, &ch, 1);
    sb->len -= 1;
}
void baseStringsSBAppendFmt(BaseStringBuilder *sb, const u8 *fmt, ...)
{
    va_list list;
    va_start(list, fmt);

    char tempBuf[1];
    i64 numWritten = vsnprintf(tempBuf, 1, fmt, list);

    BaseArenaTemp temp = baseTempBegin(&sb->arena, 1);
    {
        BaseStringBuilder tempSb = baseStringsCreateSB(temp.arena, numWritten + 1);
        {
            tempSb.len = vsnprintf(tempSb.data, tempSb.cap, fmt, list);
        }

        baseStringsSBAppendCStr(sb, tempSb.data, tempSb.len);
    }

    baseTempEnd(temp);
    va_end(list);
}
