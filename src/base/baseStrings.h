#ifndef BASE_STRING_H
#define BASE_STRING_H

#include <stdarg.h>
#include "baseCoreTypes.h"

#define STR8(CSTRING) (baseStr8((u8*)(CSTRING), strlen(CSTRING)))
#define STR8_LIT(BYTES) (baseStr8((u8*)(BYTES), ((sizeof(BYTES)) - 1)))
#define STR8_LIT_COMP(BYTES) ((str8){(u8*)(BYTES), ((sizeof(BYTES)) - 1)})
#define STR16(WCSTRING) (baseStr16((u16*)(WCSTRING), baseStr16DataLen(WCSTRING)))
#define STR16_LIT(STR) (baseStr16((u16*)(STR), BASE_ARRAY_SIZE(STR)))

typedef struct BaseStringBuilder
{
    u8* data;
    u64 cap;

    u64 len;

    //optional
    BaseArena *arena;
}BaseStringBuilder;

// i define these here myself instead of using the BASE_CREATE_LL* macros
// because i need custom behaviour for strlists
typedef struct Str8ListNode Str8ListNode;
typedef struct Str8List Str8List;
typedef struct Str8ListNode
{
	Str8ListNode *next;
	Str8ListNode *prev;
	str8 val;
}Str8ListNode;
typedef struct Str8List
{
	Str8ListNode *first;
	Str8ListNode *last;
	u64 len;
	u64 totalSize;
	u64 totalBytes;
}Str8List;

typedef struct Str8ListJoinParams
{
    str8 pre;
    str8 sep;
    str8 post;
}Str8ListJoinParams;

typedef u64 StrMatchFlags;
enum
{
    STR_MATCHFLAGS_CASE_INSENSITIVE  = (1<<0),
    STR_MATCHFLAGS_SLASH_INSENSITIVE = (1<<2),
    STR_MATCHFLAGS_FIND_LAST         = (1<<3),
};

void Str8ListPushNodeLast(Str8List *l, Str8ListNode *node);
void Str8ListPushNodeFirst(Str8List *l, Str8ListNode *node);
void Str8ListInsertNode(Str8List *l, Str8ListNode *prev, Str8ListNode *node);
void Str8ListPushLast(BaseArena *arena, Str8List *l, str8 value);
void Str8ListPushFirst(BaseArena *arena, Str8List *l, str8 value);
void Str8ListPushInsert(BaseArena *arena, Str8List *l, Str8ListNode *prev, str8 value);
void Str8ListPushLastFmt(BaseArena *arena, Str8List *l, const i8 *fmt, ...);

str8 Str8ListJoin(BaseArena *arena, Str8List *l, Str8ListJoinParams *optionals);
ArrayView Str8ListFlattenToArray(BaseArena *arena, Str8List *l);

// strings
str8 baseStr8(u8 *bytes, u64 size);
str16 baseStr16(u16 *bytes, u64 size);
str32 baseStr32(u32 *bytes, u64 size);

u64 baseStr16DataLen(u16 *str);

str8 baseStringsPushStr8Copy(BaseArena *arena, str8 str);
str8 baseStringsPushStr8FmtV(BaseArena *arena, const i8 *fmt, va_list args);
str8 baseStringsPushStr8Fmt(BaseArena *arena, const i8* fmt, ...);

bool baseStringsStrIsNullOrEmpty(str8 a);
i64 baseStringsStrCompare(str8 a, str8 b);
bool baseStringsStrEquals(str8 a, str8 b, StrMatchFlags flags);
bool baseStringsStrContains(str8 a, u8 ch);
str8 baseStringsStrSubStr8(str8 str, u64 start, u64 end);
u64 baseStringsStrFindSubStr8(str8 haystack, str8 needle, u64 start_pos, StrMatchFlags flags);
str8 baseStringsStrReplace(BaseArena *arena, str8 str, u8 old, u8 new);

str8 baseStringsStrChopPastLastSlash(str8 str);

// string builder
BaseStringBuilder baseStringsSBCreate(BaseArena *arena, u64 cap);
void baseStringsSBAppendBytes(BaseStringBuilder *sb, const u8 *bytes, u64 count);
void baseStringsSBAppendCStr(BaseStringBuilder *sb, const i8 *str, i64 strSize);
void baseStringsSBAppendStr8(BaseStringBuilder *sb, str8 str);
void baseStringsSBAppendFmt(BaseStringBuilder *sb, const i8 *fmt, ...);

// conversions
BASE_CREATE_LL_DECLS_DEFS(U16List, u16);
BASE_CREATE_LL_DECLS_DEFS(U8List, u8);

typedef struct DecodeCodePointInfo
{
    u32 codepoint;
    u64 advance;
}DecodeCodePointInfo;

DecodeCodePointInfo baseDecodeCodepointFromUtf8(u8 *bytes, u64 remainingLen);
DecodeCodePointInfo baseDecodeCodepointFromUtf16(u16 *doubles, u64 remainingLen);

// outbuf should be an array of 2 u16s
u32 Utf16FromCodepoint(u32 codepoint, u16 outBuf[2]);
// outbuf should be an array of 4 u8s
u32 Utf8FromCodepoint(u32 codepoint, u8 outBuf[4]);

str8 baseStr8FromFromStr16(BaseArena *arena, str16 str);
str16 baseStr16FromFromStr8(BaseArena *arena, str8 str);

#endif