#include "baseMemory.h"
#include "baseStrings.h"
#include "baseThreads.h"
#include <string.h>

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

    BASE_PTR_LIST_FOREACH(Str8ListNode, node, l)
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
ArrayView Str8ListFlattenToArray(BaseArena *arena, Str8List *l)
{
    ArrayView view = {0};
	view.data = baseArenaPushNoZero(arena, l->totalSize);
	view.len = l->len;
	i64 i = 0;
	BASE_PTR_LIST_FOREACH(Str8ListNode, node, l)
	{
		str8 *elem = view.data;
		elem[i] = node->val;
		i++;
	}
	return view;
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

    i64 numWritten = stbsp_vsnprintf(null, 0, (i8 *)fmt, list);

    str8 s = {0};
    s.data = baseArenaPushNoZero(arena, numWritten + 1);
    s.len = numWritten;

    stbsp_vsnprintf((i8 *)s.data, s.len + 1, (i8 *)fmt, list);
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

bool baseStringsStrIsNullOrEmpty(str8 a)
{
    return a.data == null || a.len <= 0;
}
i64 baseStringsStrCompare(str8 a, str8 b)
{
    if (a.data == null || b.data == null)
    {
        if (a.len == b.len)
        {
            return 0;
        }
        else return INT64_MAX;
    }

    return strcmp((char *)a.data, (char *) b.data);
}
bool baseStringsStrEquals(str8 a, str8 b, StrMatchFlags flags)
{
    bool result = false;
    if (a.len == b.len)
    {
        result = true;
        for(u64 i = 0; i < a.len; i++)
        {
            bool match = a.data[i] == b.data[i];
            if (flags & STR_MATCHFLAGS_CASE_INSENSITIVE)
            {
                match = (toupper(a.data[i]) == toupper(b.data[i]));
            }

            if (flags & STR_MATCHFLAGS_SLASH_INSENSITIVE)
            {
                match = ((a.data[i] == '\\' ? '/' : a.data[i])) == ((b.data[i] == '\\' ? '/' : b.data[i]));
            }

            if(!match)
            {
                return false;
            }
        }
    }

    return result;
}
bool baseStringsStrContains(str8 a, u8 ch)
{
    for(u64 i = 0; i < a.len; i++)
    {
        if(a.data[i] == ch)
        {
            return true;
        }
    }

    return false;
}
str8 baseStringsStrSubStr8(str8 str, u64 start, u64 end)
{
    u64 min = start;
    u64 max = end;
    if(max > str.len)
    {
        max = str.len;
    }

    if(min > str.len)
    {
        min = str.len;
    }
    if(min > max)
    {
        u64 swap = min;
        min = max;
        max = swap;
    }
    str.len = max - min;
    str.data += min;
    
    return str;
}
u64 baseStringsStrFindSubStr8(str8 haystack, str8 needle, u64 startPos, StrMatchFlags flags)
{
    bool found = false;
    u64 foundIndex = haystack.len;
    for(u64 i = startPos; i < haystack.len; i += 1)
    {
        if(i + needle.len <= haystack.len)
        {
            str8 substr = baseStringsStrSubStr8(haystack, i, i + needle.len);
            if(baseStringsStrEquals(substr, needle, flags))
            {
                foundIndex = i;
                found = true;
                if(!(flags & STR_MATCHFLAGS_FIND_LAST))
                {
                    break;
                }
            }
        }
    }
    return foundIndex;
}

str8 baseStringsStrChopPastLastSlash(str8 str)
{
    u64 lastSlashIndex = baseStringsStrFindSubStr8(str,
                                                   STR8_LIT("/"),
                                                   0, 
                                                   STR_MATCHFLAGS_SLASH_INSENSITIVE | STR_MATCHFLAGS_FIND_LAST);
    if (lastSlashIndex < str.len)
    {
        str.len = lastSlashIndex;
    }

    return str;
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

    i64 numWritten = stbsp_vsnprintf(null, 0, (i8 *)fmt, list);

    BaseArenaTemp temp = baseTempBegin(&sb->arena, 1);
    {
        BaseStringBuilder tempSb = baseStringsCreateSB(temp.arena, numWritten + 1);
        {
            tempSb.len = stbsp_vsnprintf((i8 *)tempSb.data, tempSb.cap, (i8 *)fmt, list);
        }

        baseStringsSBAppendCStr(sb, (i8*) tempSb.data, tempSb.len);
    }

    baseTempEnd(temp);
    va_end(list);
}
