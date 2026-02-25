#ifndef BASE_STRING_H
#define BASE_STRING_H

#include <stdarg.h>
#include "baseCoreTypes.h"
#include "baseMetagen.h"

#define STR8(CSTRING) (baseStr8((u8*)(CSTRING), strlen(CSTRING)))
#define STR8_LIT(BYTES) (baseStr8((u8*)(BYTES), ((sizeof(BYTES)) - 1)))
#define STR8_LIT_COMP(BYTES) (str8){(u8*)(BYTES), ((sizeof(BYTES)) - 1)}
#define STR8_LIT_COMP_CONST(BYTES) {(u8*)(BYTES), ((sizeof(BYTES)) - 1)}
#define STR16(WCSTRING) (baseStr16((u16*)(WCSTRING), baseStr16DataLen(WCSTRING)))
#define STR16_LIT(STR) (baseStr16((u16*)(STR), BASE_ARRAY_SIZE(STR)))

#define STR8_EMPTY (STR8_LIT(""))

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

typedef u64 StrSplitFlags;
enum
{
    STR_SPLITFLAGS_DISCARD_EMPTY = (1 << 0),
};

void Str8ListPushNodeLast(Str8List *l, Str8ListNode *node);
void Str8ListPushNodeFirst(Str8List *l, Str8ListNode *node);
void Str8ListInsertNode(Str8List *l, Str8ListNode *prev, Str8ListNode *node);
void Str8ListPushLast(Arena *arena, Str8List *l, str8 value);
void Str8ListPushFirst(Arena *arena, Str8List *l, str8 value);
void Str8ListPushInsert(Arena *arena, Str8List *l, Str8ListNode *prev, str8 value);
void Str8ListPushLastFmt(Arena *arena, Str8List *l, const i8 *fmt, ...);

void Str8ListPushListLast(Arena *arena, Str8List *l, Str8List* a);

u64 Str8ListFindFirst(Str8List *l, str8 needle, StrMatchFlags flags);
str8 Str8ListJoin(Arena *arena, Str8List *l, Str8ListJoinParams *optionals);
ArrayView Str8ListFlattenToArray(Arena *arena, Str8List *l);

// strings
str8 baseStr8(u8 *bytes, u64 size);
str16 baseStr16(u16 *bytes, u64 size);
str32 baseStr32(u32 *bytes, u64 size);

u64 baseStr16DataLen(u16 *str);

str8 Str8PushCopy(Arena *arena, str8 str);
str8 Str8PushFmtV(Arena *arena, const i8 *fmt, va_list args);
str8 Str8PushFmt(Arena *arena, const i8* fmt, ...);

bool Str8IsNullOrEmpty(str8 a);
i64 Str8Compare(str8 a, str8 b);
bool Str8Equals(str8 a, str8 b, StrMatchFlags flags);
bool Str8Contains(str8 a, u8 ch);
str8 Str8SubStr8(str8 str, u64 start, u64 end);
u64 Str8FindSubStr8(str8 haystack, str8 needle, u64 start_pos, StrMatchFlags flags);
str8 Str8Replace(Arena *arena, str8 str, u8 old, u8 new);
bool Str8StartsWith(str8 str, str8 startsWith, StrMatchFlags flags);
bool Str8EndsWith(str8 str, str8 endsWith, StrMatchFlags flags);
str8 Str8Skip(str8 str, i64 amount);
Str8List Str8Split(Arena *arena, str8 str, str8 splitWith, StrMatchFlags matchFlags, StrSplitFlags splitFlags);

str8 Str8Lower(Arena *arena, str8 str);

str8 Str8TrimStart(str8 str);
str8 Str8TrimEnd(str8 str);
str8 Str8Trim(str8 str);
str8 Str8ChopPast(str8 str, str8 past, StrMatchFlags flags);
str8 Str8ChopPastLastSlash(str8 str);

// conversions
BASE_CREATE_LL_DECLS_DEFS(U16List, u16);
BASE_CREATE_LL_DECLS_DEFS(U8List, u8);

typedef struct DecodeCodePointInfo
{
    u32 codepoint;
    u64 advance;
}DecodeCodePointInfo;

DecodeCodePointInfo baseStringsDecodeCodepointFromUtf8(u8 *bytes, u64 remainingLen);
DecodeCodePointInfo baseStringsDecodeCodepointFromUtf16(u16 *doubles, u64 remainingLen);

// outbuf should be an array of 2 u16s
u32 baseStringsUtf16FromCodepoint(u32 codepoint, u16 outBuf[2]);

// outbuf should be an array of 4 u8s
u32 baseStringsUtf8FromCodepoint(u32 codepoint, u8 outBuf[4]);

str8 Str8FromFromStr16(Arena *arena, str16 str);
str16 Str16FromFromStr8(Arena *arena, str8 str);

u64 U64FromStr8(str8 str);
i64 I64FromStr8(str8 str);
#endif