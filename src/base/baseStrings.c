#include "baseMemory.h"
#include "baseStrings.h"
#include "baseThreads.h"

void Str8ListPushNodeLast(Str8List *l, Str8ListNode *node)
{
	BaseDllNodePushLast(l->first, l->last, node);
	l->len += 1;
	l->totalSize += sizeof(node->val);
	l->totalBytes += node->val.len;
}
void Str8ListPushNodeFirst(Str8List *l, Str8ListNode *node)
{
	BaseDllNodePushFirst(l->first, l->last, node);
	l->len += 1;
	l->totalSize += sizeof(node->val);
	l->totalBytes += node->val.len;
}
void Str8ListInsertNode(Str8List *l, Str8ListNode *prev, Str8ListNode *node)
{
	BaseDllNodeInsert(l->first, l->last, prev, node);
	l->len += 1;
	l->totalSize += sizeof(node->val);
	l->totalBytes += node->val.len;
}
void Str8ListPushLast(BaseArena *arena, Str8List *l, str8 value)
{
	Str8ListNode *n = baseArenaPush(arena, sizeof(Str8ListNode));
	n->val = value;
	Str8ListPushNodeLast(l, n);
}
void Str8ListPushFirst(BaseArena *arena, Str8List *l, str8 value)
{
	Str8ListNode *n = baseArenaPush(arena, sizeof(Str8ListNode));
	n->val = value;
	Str8ListPushNodeFirst(l, n);
}
void Str8ListPushInsert(BaseArena *arena, Str8List *l, Str8ListNode *prev, str8 value)
{
	Str8ListNode *n = baseArenaPush(arena, sizeof(Str8ListNode));
	n->val = value;
	Str8ListInsertNode(l, prev, n);
}

void Str8ListPushLastFmt(BaseArena *arena, Str8List *l, const i8 *fmt, ...)
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

    u64 sepCount = (l->len > 0) ? l->len - 1 : 0;
    u64 resultTotalSize = (params.pre.len + l->totalBytes + (sepCount * params.sep.len) + params.post.len);

    str8 result = {0};
    result.data = baseArenaPushNoZero(arena, resultTotalSize + 1);
    result.len = resultTotalSize;

    u8 *destPtr = result.data;
    BASE_MEMCPY(destPtr, params.pre.data, params.pre.len);

    destPtr += params.pre.len;

    BASE_LIST_FOREACH(Str8ListNode, node, l)
    {
        BASE_MEMCPY(destPtr, node->val.data, node->val.len);

        destPtr += node->val.len;
        if (node != l->last)
        {
            BASE_MEMCPY(destPtr, params.sep.data, params.sep.len);
            destPtr += params.sep.len;
        }
    }

    BASE_MEMCPY(destPtr, params.post.data, params.post.len);
    destPtr += params.post.len;

    *destPtr = '\0';

    return result;
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
str8 baseStringsPushStr8FmtV(BaseArena *arena, const i8 *fmt, va_list args)
{
    va_list list;
    va_copy(list, args);

    char tempBuf[1];
    i64 numWritten = vsnprintf(tempBuf, 1, (i8 *)fmt, list);

    str8 s = {0};
    s.data = baseArenaPushNoZero(arena, numWritten + 1);
    s.len = numWritten;

    vsnprintf((i8 *)s.data, s.len + 1, (i8 *)fmt, list);
    return s;
}
str8 baseStringsPushStr8Fmt(BaseArena *arena, const i8* fmt, ...)
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
void baseStringsSBAppendCStr(BaseStringBuilder *sb, const i8 *str, i64 strSize)
{
    u64 strLength = (strSize == -1) ? strlen((i8 *)str) : strSize;
    baseStringsSBAppendBytes(sb, (u8*) str, strLength + 1); //+ 1 for coppying '\0'
    sb->len -= 1; //since we coppied the '\0' over
}
void baseStringsSBAppendStr8(BaseStringBuilder *sb, str8 str)
{
    baseStringsSBAppendBytes(sb, str.data, str.len);

    u8 ch = '\0';
    baseStringsSBAppendBytes(sb, &ch, 1);
    sb->len -= 1;
}
void baseStringsSBAppendFmt(BaseStringBuilder *sb, const i8 *fmt, ...)
{
    va_list list;
    va_start(list, (i8 *)fmt);

    char tempBuf[1];
    i64 numWritten = vsnprintf(tempBuf, 1, (i8 *)fmt, list);

    BaseArenaTemp temp = baseTempBegin(&sb->arena, 1);
    {
        BaseStringBuilder tempSb = baseStringsCreateSB(temp.arena, numWritten + 1);
        {
            tempSb.len = vsnprintf((i8 *)tempSb.data, tempSb.cap, (i8 *)fmt, list);
        }

        baseStringsSBAppendCStr(sb, (i8*) tempSb.data, tempSb.len);
    }

    baseTempEnd(temp);
    va_end(list);
}
