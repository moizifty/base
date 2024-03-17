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

void Str8ListPushNodeLast(Str8List *l, Str8ListNode *node);
void Str8ListPushNodeFirst(Str8List *l, Str8ListNode *node);
void Str8ListInsertNode(Str8List *l, Str8ListNode *prev, Str8ListNode *node);
void Str8ListPushLast(BaseArena *arena, Str8List *l, str8 value);
void Str8ListPushFirst(BaseArena *arena, Str8List *l, str8 value);
void Str8ListPushInsert(BaseArena *arena, Str8List *l, Str8ListNode *prev, str8 value);
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