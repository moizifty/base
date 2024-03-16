#include "baseMemory.h"
#include "baseStrings.h"
#include "baseThreads.h"

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

void baseStringsSBAppendCStr(BaseStringBuilder *sb, const u8 *str, i64 strSize)
{
    u64 strLength = (strSize == -1) ? strlen(str) : strSize;

    if((sb->len + strLength + 1) > sb->cap)
    {
        baseArenaPush(sb->arena, strLength + 1);
        sb->cap += strLength + 1;
    }

    strncat(sb->data, str, strLength);
    sb->len += strLength;
    sb->data[sb->len] = '\0';
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