#include "baseCore.h"
#include "baseMemory.h"
#include "baseStrings.h"
#include "baseThreads.h"
#include <string.h>

void Str8ListPushNodeLast(Str8List *l, Str8ListNode *node)
{
	BasePtrListNodePushLast(l, node);
	l->totalSize += sizeof(node->val);
	l->totalBytes += node->val.len;
}
void Str8ListPushNodeFirst(Str8List *l, Str8ListNode *node)
{
	BasePtrListNodePushFirst(l, node);
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
void Str8ListPushLast(Arena *arena, Str8List *l, str8 value)
{
	Str8ListNode *n = arenaPush(arena, sizeof(Str8ListNode));
	n->val = value;
	Str8ListPushNodeLast(l, n);
}
void Str8ListPushFirst(Arena *arena, Str8List *l, str8 value)
{
	Str8ListNode *n = arenaPush(arena, sizeof(Str8ListNode));
	n->val = value;
	Str8ListPushNodeFirst(l, n);
}
void Str8ListPushInsert(Arena *arena, Str8List *l, Str8ListNode *prev, str8 value)
{
	Str8ListNode *n = arenaPush(arena, sizeof(Str8ListNode));
	n->val = value;
	Str8ListInsertNode(l, prev, n);
}

void Str8ListPopNodeLast(Str8List *l)
{
    if (BASE_ANY_PTR(l))
    {
        l->totalBytes -= l->last->val.len;
        l->totalSize -= sizeof(l->last->val);
	    BasePtrListNodeRemove(l, l->last);
    }
}

void Str8ListPushLastFmt(Arena *arena, Str8List *l, const i8 *fmt, ...)
{
    va_list list;
    va_start(list, fmt);

    str8 s = Str8PushFmtV(arena, fmt, list);

    va_end(list);

    Str8ListPushLast(arena, l, s);
}

void Str8ListPushListLast(Arena *arena, Str8List *l, Str8List* a)
{
    BASE_PTR_LIST_FOREACH(Str8ListNode, n, a)
    {
        Str8ListPushLast(arena, l, n->val);
    }
}

u64 Str8ListFindFirst(Str8List *l, str8 needle, StrMatchFlags flags)
{
    u64 index = 0;
    BASE_PTR_LIST_FOREACH_INDEX(Str8ListNode, node, l, index)
    {
        if (Str8Equals(node->val, needle, flags))
        {
            break;
        }
    }

    return index;
}
str8 Str8ListJoin(Arena *arena, Str8List *l, Str8ListJoinParams *optionals)
{
    Str8ListJoinParams params = {0};
    if (optionals != null)
    {
        params = *optionals;
    }

    u64 sepCount = (l->len > 0) ? l->len - 1 : 0;
    u64 resultTotalSize = (params.pre.len + l->totalBytes + (sepCount * params.sep.len) + params.post.len);

    str8 result = {0};
    result.data = arenaPushNoZero(arena, resultTotalSize + 1);
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
ArrayView Str8ListFlattenToArray(Arena *arena, Str8List *l)
{
    ArrayView view = {0};
	view.data = arenaPushNoZero(arena, l->totalSize);
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

u64 baseStr16DataLen(u16 *str)
{
    u64 i = 0;
    for(; str[i] != '\0'; i++);
    return i;
}

str8 Str8PushCopy(Arena *arena, str8 str)
{
    str8 s = {0};
    s.data = arenaPushNoZero(arena, str.len + 1);
    s.len = str.len;

    BASE_MEMCPY(s.data, str.data, s.len);
    s.data[s.len] = '\0';

    return s;
}
str8 Str8PushFmtV(Arena *arena, const i8 *fmt, va_list args)
{
    va_list list;
    va_copy(list, args);

    i64 numWritten = stbsp_vsnprintf(null, 0, (i8 *)fmt, list);

    str8 s = {0};
    s.data = arenaPushNoZero(arena, numWritten + 1);
    s.len = numWritten;

    stbsp_vsnprintf((i8 *)s.data, (int)s.len + 1, (i8 *)fmt, list);
    return s;
}
str8 Str8PushFmt(Arena *arena, const i8* fmt, ...)
{
    va_list list;
    va_start(list, fmt);

    str8 s = Str8PushFmtV(arena, fmt, list);

    va_end(list);
    return s;
}

bool Str8IsNullOrEmpty(str8 a)
{
    return a.data == null || a.len <= 0;
}
i64 Str8Compare(str8 a, str8 b)
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

bool Str8Equals(str8 a, str8 b, StrMatchFlags flags)
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
bool Str8Contains(str8 a, str8 needle, StrMatchFlags flags)
{
    return Str8FindSubStr8(a, needle, 0, flags) != a.len;
}
str8 Str8SubStr8(str8 str, u64 start, u64 end)
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
u64 Str8FindSubStr8(str8 haystack, str8 needle, u64 startPos, StrMatchFlags flags)
{
    bool found = false;
    u64 foundIndex = haystack.len;
    for(u64 i = startPos; i < haystack.len; i += 1)
    {
        if(i + needle.len <= haystack.len)
        {
            str8 substr = Str8SubStr8(haystack, i, i + needle.len);
            if(Str8Equals(substr, needle, flags))
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
str8 Str8Replace(Arena *arena, str8 str, u8 old, u8 new)
{
    str8 ret = Str8PushFmt(arena, "%S", str);

    for(u64 i = 0; i < ret.len; i++)
    {
        if(ret.data[i] == old)
        {
            ret.data[i] = new;
        }
    }

    return ret;
}
bool Str8StartsWith(str8 str, str8 startsWith, StrMatchFlags flags)
{
    if(str.len < startsWith.len)
    {
        return false;
    }

    str8 a = {.data = str.data, .len = startsWith.len};

    return Str8Equals(a, startsWith, flags);
}
bool Str8EndsWith(str8 str, str8 endsWith, StrMatchFlags flags)
{
    if(str.len < endsWith.len)
    {
        return false;
    }

    str8 a = {.data = (str.data + str.len) - endsWith.len, .len = endsWith.len};

    return Str8Equals(a, endsWith, flags);
}
str8 Str8Skip(str8 str, i64 amount)
{
    str8 a = str;

    if (!BASE_NULL_OR_EMPTY(a))
    {
        if ((i64)a.len >= amount)
        {
            a.data += amount;
            a.len -= amount;
        }
    }

    return a;
}
Str8List Str8Split(Arena *arena, str8 str, str8 splitWith, StrMatchFlags matchFlags, StrSplitFlags splitFlags)
{
    Str8List ret = {0};

    u64 lastIndex = 0;
    str8 remainingStr = str;

    bool noCopy = splitFlags & STR_SPLITFLAGS_NO_COPY;
    while(lastIndex < remainingStr.len)
    {
        u64 swIndex = Str8FindSubStr8(remainingStr, splitWith, 0, matchFlags);
        if (swIndex == remainingStr.len)
        {
            break;
        }

        str8 before =
        {
            .data = remainingStr.data,
            .len = swIndex,
        };

        if (BASE_ANY(before) || !(splitFlags & STR_SPLITFLAGS_DISCARD_EMPTY))
        {
            Str8ListPushLast(arena, &ret, (noCopy) ? before : Str8PushCopy(arena, before));
        }


        remainingStr.data += swIndex + splitWith.len;
        remainingStr.len -= (splitWith.len + swIndex);
    }

    if (BASE_ANY(remainingStr) || !(splitFlags & STR_SPLITFLAGS_DISCARD_EMPTY))
    {
        Str8ListPushLast(arena, &ret, (noCopy) ? remainingStr : Str8PushCopy(arena, remainingStr));

    }
    
    return ret;
}
str8 Str8TrimStart(str8 str)
{
    str8 a = str;
    if (!BASE_NULL_OR_EMPTY(a))
    {
        while ((a.len > 0) && (a.data[0] == ' ' || a.data[0] == '\t' || a.data[0] == '\r' || a.data[0] == '\n' || a.data[0] == '\f'))
        {
            a.data++;
            a.len--;
        }
    }

    return a;
}
str8 Str8TrimEnd(str8 str)
{
    str8 a = str;
    if (!BASE_NULL_OR_EMPTY(a))
    {
        while ((a.len > 0) && (a.data[a.len - 1] == ' ' || a.data[a.len - 1] == '\t' || a.data[a.len - 1] == '\r' || a.data[a.len - 1] == '\n' || a.data[a.len - 1] == '\f'))
        {
            a.len--;
        }
    }

    return a;
}
str8 Str8Trim(str8 str)
{
    return Str8TrimEnd(Str8TrimStart(str));
}

str8 Str8ChopPast(str8 str, str8 past, StrMatchFlags flags)
{
    u64 lastIndex = Str8FindSubStr8(str,
                                    past,
                                    0, 
                                    flags);
    if (lastIndex < str.len)
    {
        str.len = lastIndex;
    }

    return str;
}

str8 Str8ChopBefore(str8 str, str8 before, StrMatchFlags flags)
{
    u64 lastIndex = Str8FindSubStr8(str,
                                    before,
                                    0, 
                                    flags);
    if (lastIndex < str.len)
    {
        str.data += lastIndex + 1;
        str.len = str.len - (lastIndex + 1);
    }

    return str;
}

str8 Str8ChopPastLastSlash(str8 str)
{
    return Str8ChopPast(str, STR8_LIT("/"), STR_MATCHFLAGS_SLASH_INSENSITIVE|STR_MATCHFLAGS_FIND_LAST);
}

str8 Str8Lower(Arena *arena, str8 str)
{
    str8 ret = {0};
    ret.len = str.len;
    ret.data = arenaPushArray(arena, u8, ret.len);

    for(u64 i = 0; i < ret.len; i++)
    {
        if (BASE_ISALPHA(str.data[i]) && str.data[i] <= 'Z')
        {
            ret.data[i] = 'a' + (str.data[i] - 'A');
        }
        else
        {
            ret.data[i] = str.data[i];
        }
    }

    return ret;
}

// conversions
DecodeCodePointInfo baseStringsDecodeCodepointFromUtf8(u8 *bytes, u64 remainingLen)
{
    BASE_UNUSED_PARAM(remainingLen);

    DecodeCodePointInfo dp = {0};
    if (bytes[0] < 0b1000'0000)
    {
        dp.advance = 1;
        dp.codepoint = bytes[0];
    }
    else if(bytes[0] < 0b1110'0000)
    {
        u8 b1 = (bytes[0] & 0b000'11111);
        u8 b2 = (bytes[1] & 0b00'111111);

        u32 codepoint = (b1 << 6) | b2;

        dp.advance = 2;
        dp.codepoint = codepoint;
    }
    else if(bytes[0] < 0b11110'000)
    {
        u8 b1 = (bytes[0] & 0b0000'1111);
        u8 b2 = (bytes[1] & 0b00'111111);
        u8 b3 = (bytes[2] & 0b00'111111);

        u32 codepoint = (b1 << 12) | (b2 << 6) | b3;

        dp.advance = 3;
        dp.codepoint = codepoint;
    }
    else if(bytes[0] < 0b111110'00)
    {
        u8 b1 = (bytes[0] & 0b00000'111);
        u8 b2 = (bytes[1] & 0b00'111111);
        u8 b3 = (bytes[2] & 0b00'111111);
        u8 b4 = (bytes[3] & 0b00'111111);

        u32 codepoint = (b1 << 18) | (b2 << 12) | (b3 << 6) | b4;

        dp.advance = 4;
        dp.codepoint = codepoint;
    }

    return dp;
}
DecodeCodePointInfo baseStringsDecodeCodepointFromUtf16(u16 *doubles, u64 remainingLen)
{
    BASE_UNUSED_PARAM(remainingLen);
    
    DecodeCodePointInfo dp = {0};
    dp.advance = 1;
    dp.codepoint = (u32)doubles[0];
    if((doubles[0] >= 0xd800) && (doubles[0] < 0xdc00) && (doubles[1] >= 0xdc00) && (doubles[1] < 0xe000))
    {
        u16 b1 = (doubles[0] - 0xD800) << 10;
        u16 b2 = (doubles[1] - 0xDC00);

        dp.codepoint = (b1 | b2) + 0x10000;
        dp.advance = 2;
    }

    return dp;
}

// outbuf should be an array of 2 u16s
u32 baseStringsUtf16FromCodepoint(u32 codepoint, u16 outBuf[2])
{
    u32 encodingLength = 0;

    if (codepoint < 0x10000)
    {
        outBuf[0] = (u16)codepoint;
        encodingLength = 1;
    }
    else
    {
        u32 a = codepoint - 0x10000;
        outBuf[0] = (u16) (0xD800 + (a >> 10));
        outBuf[1] = (u16) (0xDC00 + (a & (0b00000'00000'11111'11111)));
        encodingLength = 2;
    }

    return encodingLength;
}

// outbuf should be an array of 4 u8s
u32 baseStringsUtf8FromCodepoint(u32 codepoint, u8 outBuf[4])
{
    u32 encodingLength = 0;

    if (codepoint < 0b1000'0000)
    {
        outBuf[0] = (u8)codepoint;
        encodingLength = 1;
    }
    else if(codepoint <= 0b11111'111111)
    {
        outBuf[0] = (u8) (0b110'00000 | (codepoint >> 6));
        outBuf[1] = (u8) (0b10'000000 | ((codepoint & 0b00000'111111)));
        encodingLength = 2;
    }
    else if(codepoint <= 0xffff)
    {
        outBuf[0] = (u8) (0b1110'0000 | (codepoint >> 12));
        outBuf[1] = (u8) (0b10'000000 | ((codepoint >> 6) & 0b111'111));
        outBuf[2] = (u8) (0b10'000000 | ((codepoint & 0b111'111)));
        encodingLength = 3;
    }
    else if(codepoint <= 0x10FFFF)
    {
        outBuf[0] = (u8) (0b11110'000 | (codepoint >> 18));
        outBuf[1] = (u8) (0b10'000000 | ((codepoint >> 12) & 0b111'111));
        outBuf[2] = (u8) (0b10'000000 | ((codepoint >> 6) & 0b111'111));
        outBuf[3] = (u8) (0b10'000000 | ((codepoint & 0b111'1111)));
        encodingLength = 4;
    }

    return encodingLength;
}

str8 Str8FromFromStr16(Arena *arena, str16 str)
{
    str8 outStr = {0};
    U8List utf8bytes = {0}; 
    u16 *doubles = str.data;

    ArenaTemp temp = baseTempBegin(&arena, 1);
    {
        u64 strByteLength = 0;
        for(u64 i = 0; i < str.len;)
        {
            DecodeCodePointInfo dp = baseStringsDecodeCodepointFromUtf16(doubles + i, str.len - i);
            i += dp.advance;

            u8 utf8Buf[4] = {0};
            u32 encodingLength = baseStringsUtf8FromCodepoint(dp.codepoint, utf8Buf);
            strByteLength += encodingLength;

            switch(encodingLength)
            {
                case 1:
                {
                    U8ListPushLast(temp.arena, &utf8bytes, utf8Buf[0]);
                }break;
                case 2:
                {
                    U8ListPushLast(temp.arena, &utf8bytes, utf8Buf[0]);
                    U8ListPushLast(temp.arena, &utf8bytes, utf8Buf[1]);
                }break;
                case 3:
                {
                    U8ListPushLast(temp.arena, &utf8bytes, utf8Buf[0]);
                    U8ListPushLast(temp.arena, &utf8bytes, utf8Buf[1]);
                    U8ListPushLast(temp.arena, &utf8bytes, utf8Buf[2]);
                }break;
                case 4:
                {
                    U8ListPushLast(temp.arena, &utf8bytes, utf8Buf[0]);
                    U8ListPushLast(temp.arena, &utf8bytes, utf8Buf[1]);
                    U8ListPushLast(temp.arena, &utf8bytes, utf8Buf[2]);
                    U8ListPushLast(temp.arena, &utf8bytes, utf8Buf[3]);
                }break;
            }
        }

        U8ListPushLast(temp.arena, &utf8bytes, 0);

        outStr.data = U8ListFlattenToArray(arena, &utf8bytes).data;
        outStr.len = strByteLength;
    }
    baseTempEnd(temp);

    return outStr;
}
str16 Str16FromFromStr8(Arena *arena, str8 str)
{
    str16 outStr = {0};
    U16List utf16bytes = {0}; 
    u8 *bytes = str.data;

    ArenaTemp temp = baseTempBegin(&arena, 1);
    {
        u64 strByteLength = 0;
        for(u64 i = 0; i < str.len;)
        {
            DecodeCodePointInfo dp = baseStringsDecodeCodepointFromUtf8(bytes + i, str.len - i);
            i += dp.advance;

            u16 utf16Buf[2] = {0};
            u32 encodingLength = baseStringsUtf16FromCodepoint(dp.codepoint, utf16Buf);
            strByteLength += encodingLength;

            U16ListPushLast(temp.arena, &utf16bytes, utf16Buf[0]);
            if(encodingLength > 1) U16ListPushLast(temp.arena, &utf16bytes, utf16Buf[1]);
        }

        U16ListPushLast(temp.arena, &utf16bytes, 0);

        outStr.data = U16ListFlattenToArray(arena, &utf16bytes).data;
        outStr.len = strByteLength;
    }
    baseTempEnd(temp);

    return outStr;
}


u64 U64FromStr8(str8 str)
{
    u64 num = 0;
    if (!BASE_NULL_OR_EMPTY(str))
    {
        if((str.data[0] == '0') && (str.data[1] == 'x'))
        {
            //hex number
            str = Str8Skip(str, 2);

            for(u64 i = 0; i < str.len; i++)
            {
                num = (num * 16) + (u8)baseCharHexDigitToU8(str.data[i]);
            }
        }
        else if((str.data[0] == '0') && (str.data[1] == 'b'))
        {
            //binary number
            str = Str8Skip(str, 2);

            for(u64 i = 0; i < str.len; i++)
            {
                num = (num * 2) + (u8)baseCharBinDigitToU8(str.data[i]);
            }
        }
        else
        {
            for(u64 i = 0; i < str.len; i++)
            {
                num = (num * 10) + (u8)baseCharDigitToU8(str.data[i]);
            }
        }
    }

    return num;
}

i64 I64FromStr8(str8 str)
{
    i8 sign = 1;
    i64 num = 0;
    if (!BASE_NULL_OR_EMPTY(str))
    {
        if (str.data[0] == '-') sign = -1;

        num = (i64)U64FromStr8(str);
    }

    return num * sign;
}
