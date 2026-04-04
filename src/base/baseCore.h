#ifndef BASE_CORE_H
#define BASE_CORE_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "baseCoreTypes.h"
#include "thirdparty/ts_stb_sprintf.h"

// General

#define BASE_OFFSETOF(s, memb) ((u64)(&(((s *)0)->memb)))
#define BASE_ALIGNOF(T) (_Alignof(T))
#define BASE_BYTES(NUM) ((u64)(NUM))
#define BASE_KILOBYTES(NUM) ((u64)(BASE_BYTES(NUM)) * 1024u)
#define BASE_MEGABYTES(NUM) ((u64)(BASE_KILOBYTES(NUM)) * 1024u)
#define BASE_GIGABYTES(NUM) ((u64)(BASE_MEGABYTES(NUM)) * 1024u)

#define BASE_IS_POWER_OF_2(NUM) ((NUM) & ((NUM) - 1))
#define BASE_NUM_BETWEEN(X, S, E)     (((X) >= (S)) && ((X) <= (E)))
#define BASE_ARRAY_SIZE(ARR)  ((sizeof(ARR)) / (sizeof((ARR[0]))))
#define BASE_MAX(A, B)  (((A) > (B)) ? (A) : (B))

#define BASE_CLAMP(V, MIN, MAX)  (((V) <= (MIN)) ? (MIN) : ((V) >= MAX) ? (MAX) : (V))

#define BASE_UNUSED_PARAM(P)	((void)(P))

// Bitwise
#define BASE_SET_FLAG(n, f)    ((n) |= (f));

// printing
#define BASE_TERMINAL_ESC_CODE   			"\033"
#define BASE_TERMINAL_RESET_CODE   			BASE_TERMINAL_ESC_CODE "[0m"
#define BASE_TERMINAL_BOLD_CODE   			BASE_TERMINAL_ESC_CODE "[1m"
#define BASE_TERMINAL_UNDERLINE_CODE   		BASE_TERMINAL_ESC_CODE "[4m"

#define BASE_TERMINAL_FG_RED_CODE  			BASE_TERMINAL_ESC_CODE "[31m"
#define BASE_TERMINAL_FG_BLUE_CODE  		BASE_TERMINAL_ESC_CODE "[34m"
#define BASE_TERMINAL_FG_ORANGE_1_CODE  	BASE_TERMINAL_ESC_CODE "[38;2;255;127;80m"
#define BASE_TERMINAL_FG_GREEN_CODE  		BASE_TERMINAL_ESC_CODE "[38;2;156;254;220m"

#define TERMINAL_BG_RED_CODE  				BASE_TERMINAL_ESC_CODE "[41m"

#define baseColPrintf(FMT,...) (baseColFprintf(stdout, FMT, ##__VA_ARGS__))
#define baseColEPrintf(FMT,...) (baseColFprintf((stderr), (FMT), ##__VA_ARGS__))
#define basePrintf baseColPrintf
#define baseEPrintf baseColEPrintf

#define baseColPrintfV(FMT, VA) (baseColFprintfV(stdout, FMT, VA))
#define baseColEPrintfV(FMT, VA) (baseColFprintfV((stderr), (FMT), VA))
#define basePrintfV baseColPrintfV
#define baseEPrintfV baseColEPrintfV

// idk other stuff
#define BASE_ISALPHA(c) isalpha((int)(c))
#define BASE_ISDIGIT(c) isdigit((int)(c))
#define BASE_ISHEXDIGIT(c) isxdigit((int)(c))

// memory
#define BASE_MEMCPY memcpy
#define BASE_MEMCPY_BE baseMemcpyBigEndian
#define BASE_MEMCMP memcmp
#define BASE_MEMSET memset
#define BASE_MEMZERO(DEST, SZ) BASE_MEMSET((DEST), 0, (SZ))

// lists, and stuff
#define BASE_LIST_FOREACH_EX(NODETYPE, NAME, LIST, next)			for(NODETYPE *NAME = (LIST).first; (NAME) != null; (NAME) = (NAME)->next)
#define BASE_PTR_LIST_FOREACH_EX(NODETYPE, NAME, LIST, next)		BASE_LIST_FOREACH_EX(NODETYPE, NAME, *(LIST), next)

#define BASE_LIST_REVFOREACH_EX(NODETYPE, NAME, LIST, prev)			for(NODETYPE *NAME = (LIST).last; (NAME) != null; (NAME) = (NAME)->prev)
#define BASE_PTR_LIST_REVFOREACH_EX(NODETYPE, NAME, LIST, prev)		BASE_LIST_REVFOREACH_EX(NODETYPE, NAME, *(LIST), prev)	

#define BASE_LIST_FOREACH_INDEX_EX(NODETYPE, NAME, LIST, INDEX, next)			for(NODETYPE *NAME = (LIST).first; (NAME) != null; (NAME) = (NAME)->next, (INDEX)++)
#define BASE_PTR_LIST_FOREACH_INDEX_EX(NODETYPE, NAME, LIST, INDEX, next)		BASE_LIST_FOREACH_INDEX_EX(NODETYPE, NAME, *(LIST), INDEX, next)

#define BASE_LIST_FOREACH(NODETYPE, NAME, LIST)			BASE_LIST_FOREACH_EX(NODETYPE, NAME, LIST, next)
#define BASE_PTR_LIST_FOREACH(NODETYPE, NAME, LIST)		BASE_PTR_LIST_FOREACH_EX(NODETYPE, NAME, LIST, next)

#define BASE_LIST_REVFOREACH(NODETYPE, NAME, LIST)			BASE_LIST_REVFOREACH_EX(NODETYPE, NAME, LIST, prev)		
#define BASE_PTR_LIST_REVFOREACH(NODETYPE, NAME, LIST)		BASE_PTR_LIST_REVFOREACH_EX(NODETYPE, NAME, LIST, prev)	

#define BASE_LIST_FOREACH_INDEX(NODETYPE, NAME, LIST, INDEX)			BASE_LIST_FOREACH_INDEX_EX(NODETYPE, NAME, LIST, INDEX, next)
#define BASE_PTR_LIST_FOREACH_INDEX(NODETYPE, NAME, LIST, INDEX)		BASE_PTR_LIST_FOREACH_INDEX_EX(NODETYPE, NAME, LIST, INDEX, next)


#define CheckNull(p) ((p)==0)
#define SetNull(p) ((p)=0)

#define BaseDllNodeInsertEx(f, l, p, n, prev, next)  \
((CheckNull(f)) ? ((f) = (l) = (n), (n)->prev = (n)->next = null) \
: ((CheckNull(p)) ? ((n)->next = (f), (n)->prev = null, (f)->prev = (n), (f) = (n)) \
: ((!CheckNull((p)->next)) ? (p)->next->prev = (n) : 0), ((n)->next = (p)->next, (n)->prev = (p), (p)->next = (n), (((p) == (l)) ? (l) = (n) : 0))))
#define BaseDllRemoveEx(f,l,n, prev, next) (((f)==(n))?\
((f)=(f)->next, (CheckNull(f) ? (SetNull(l)) : SetNull((f)->prev))):\
((l)==(n))?\
((l)=(l)->prev, (CheckNull(l) ? (SetNull(f)) : SetNull((l)->next))):\
((CheckNull((n)->next) ? (0) : ((n)->next->prev=(n)->prev)),\
(CheckNull((n)->prev) ? (0) : ((n)->prev->next=(n)->next))))

#define BaseDllNodeInsert(f, l, p, n)   BaseDllNodeInsertEx(f, l, p, n, prev, next)
#define BaseDllNodePushLast(f, l, n)    BaseDllNodeInsertEx(f, l, l, n, prev, next)
#define BaseDllNodePushFirst(f, l, n)   BaseDllNodeInsertEx(l, f, f, n, next, prev)
#define BaseDllRemove(f,l,n)            BaseDllRemoveEx(f,l,n, prev,next)

#define BaseListNodePushLastEx(list, n, prev, next)   (BaseDllNodeInsertEx((list).first, (list).last, (list).last, n, prev, next), (list).len++)
#define BasePtrListNodePushLastEx(list, n, prev, next)   (BaseListNodePushLastEx(*list, n, prev, next))

#define BaseListNodePushFirstEx(list, n, prev, next)   (BaseDllNodeInsertEx((list).last, (list).first, (list).first, n, next, prev), (list).len++)
#define BasePtrListNodePushFirstEx(list, n, prev, next) (BaseListNodePushFirstEx(*(list), n, next, prev))

#define BaseListNodeInsertEx(list, p, n, prev, next)   (BaseDllNodeInsertEx((list).first, (list).last, p, n, prev, next), (list).len++)
#define BasePtrListNodeInsertEx(list, p, n, prev, next)   (BaseListNodeInsertEx(*(list), p, n, prev, next))

#define BaseListNodeRemoveEx(list, n, prev, next)   (BaseDllRemoveEx((list).first, (list).last, n, prev, next), (list).len--)
#define BasePtrListNodeRemoveEx(list, n, prev, next)   (BaseListNodeRemoveEx(*(list), n, prev, next))

#define BaseListNodePushLast(list, n)   (BaseListNodePushLastEx((list), (n), prev, next))
#define BasePtrListNodePushLast(list, n)   (BasePtrListNodePushLastEx((list), (n), prev, next))

#define BaseListNodePushFirst(list, n)   (BaseListNodePushFirstEx((list), (n), next, prev))
#define BasePtrListNodePushFirst(list, n)   (BasePtrListNodePushFirstEx((list), (n), next, prev))

#define BaseListNodeInsert(list, p, n)   (BaseListNodeInsertEx((list), p, n, prev, next))
#define BasePtrListNodeInsert(list, p, n)   (BasePtrListNodeInsertEx((list), p, n, prev, next))

#define BaseListNodeRemove(list, n)   (BaseListNodeRemoveEx((list), n, prev, next))
#define BasePtrListNodeRemove(list, n)   (BasePtrListNodeRemoveEx((list), n, prev, next))

#define BASE_ANY_PTR(pL)     (((pL) != NULL) && (pL)->len != 0)
#define BASE_ANY(L)     ((L).len != 0)
#define BASE_NULL_OR_EMPTY(L) (((L).data == null) || !BASE_ANY(L))

#define BASE_PTR_LL_FIRST(pLL, DEFVAL)     (BASE_ANY_PTR(pLL) ? (pLL)->first->val : DEFVAL)
#define BASE_LL_FIRST(LL, DEFVAL)          (BASE_ANY(LL) ? (LL).first->val : DEFVAL)

#define BASE_PTR_LL_LAST(pLL, DEFVAL)      (BASE_ANY_PTR(pLL) ? (pLL)->last->val : DEFVAL)
#define BASE_LL_LAST(LL, DEFVAL)           (BASE_ANY(LL) ? (LL).last->val : DEFVAL)

#define BASE_CREATE_LL_JUST_LIST_DECLS_EX(NAME, NODENAME, ELEM) \
typedef struct NAME NAME; \
typedef struct NODENAME NODENAME; \
typedef struct NAME \
{ \
	NODENAME *first; \
	NODENAME *last; \
	u64 len; \
	u64 totalSize; \
}NAME; \
typedef bool (*NAME##FirstFunc)(NODENAME *node); \
typedef bool (*NAME##FirstCaptureFunc)(NODENAME *node, void *capture); \
inline NODENAME *NAME##First(NAME *l, NAME##FirstFunc func); \
inline NODENAME *NAME##FirstCapture(NAME *l, void *capture, NAME##FirstCaptureFunc func); \
typedef bool (*NAME##WhereFunc)(NODENAME *node); \
inline NAME NAME##WhereList(Arena *arena, NAME *l, NAME##WhereFunc func); \
void NAME##PushNodeLast(NAME *l, NODENAME *node); \
void NAME##PushNodeFirst(NAME *l, NODENAME *node); \
void NAME##InsertNode(NAME *l, NODENAME *prev, NODENAME *node); \
void NAME##PushLast(struct Arena *arena, NAME *l, ELEM value); \
void NAME##PushFirst(struct Arena *arena, NAME *l, ELEM value); \
void NAME##PushInsert(struct Arena *arena, NAME *l, NODENAME *prev, ELEM value); \
ArrayView NAME##FlattenToArray(struct Arena *arena, NAME *l); \

#define BASE_CREATE_LL_JUST_NODE_DECLS_EX(NAME, NODENAME, ELEM) \
typedef struct NAME NAME; \
typedef struct NODENAME NODENAME; \
typedef struct NODENAME \
{ \
	NODENAME *next; \
	NODENAME *prev; \
	ELEM val; \
}NODENAME;

#define BASE_CREATE_LL_DECLS_EX(NAME, NODENAME, ELEM) \
BASE_CREATE_LL_JUST_LIST_DECLS_EX(NAME, NODENAME, ELEM) \
BASE_CREATE_LL_JUST_NODE_DECLS_EX(NAME, NODENAME, ELEM)  \

#define BASE_CREATE_LL_JUST_LIST_DEFS_EX(NAME, NODENAME, ELEM) \
inline NODENAME *NAME##First(NAME *l, NAME##FirstFunc func) \
{\
	BASE_PTR_LIST_FOREACH(NODENAME, node, l) \
	{ \
		if (func(node)) \
		{ \
			return node;\
		} \
	} \
	return null; \
}\
inline NODENAME *NAME##FirstCapture(NAME *l, void *capture, NAME##FirstCaptureFunc func) \
{\
	BASE_PTR_LIST_FOREACH(NODENAME, node, l) \
	{ \
		if (func(node, capture)) \
		{ \
			return node;\
		} \
	} \
	return null; \
}\
inline NAME NAME##WhereList(Arena *arena, NAME *l, NAME##WhereFunc func) \
{ \
	NAME ret = {0};\
    BASE_PTR_LIST_FOREACH(NODENAME, node, l)\
    {\
        if (func(node))\
        {\
            NAME##PushLast(arena, &ret, node->val);\
        }\
    }\
    return ret;\
} \
inline void NAME##PushNodeLast(NAME *l, NODENAME *node) \
{ \
	BaseDllNodePushLast(l->first, l->last, node); \
	l->len += 1; \
	l->totalSize += sizeof(node->val); \
} \
inline void NAME##PushNodeFirst(NAME *l, NODENAME *node) \
{ \
	BaseDllNodePushFirst(l->first, l->last, node); \
	l->len += 1; \
	l->totalSize += sizeof(node->val); \
} \
inline void NAME##InsertNode(NAME *l, NODENAME *prev, NODENAME *node) \
{ \
	BaseDllNodeInsert(l->first, l->last, prev, node); \
	l->len += 1; \
	l->totalSize += sizeof(node->val); \
} \
inline void NAME##PushLast(struct Arena *arena, NAME *l, ELEM value) \
{ \
	NODENAME *n = arenaPush(arena, sizeof(NODENAME)); \
	n->val = value; \
	NAME##PushNodeLast(l, n); \
} \
inline void NAME##PushFirst(struct Arena *arena, NAME *l, ELEM value) \
{ \
	NODENAME *n = arenaPush(arena, sizeof(NODENAME)); \
	n->val = value; \
	NAME##PushNodeFirst(l, n); \
} \
inline void NAME##PushInsert(struct Arena *arena, NAME *l, NODENAME *prev, ELEM value) \
{ \
	NODENAME *n = arenaPush(arena, sizeof(NODENAME)); \
	n->val = value; \
	NAME##InsertNode(l, prev, n); \
} \
inline ArrayView NAME##FlattenToArray(struct Arena *arena, NAME *l) \
{ \
	ArrayView view = {0}; \
	view.data = arenaPushNoZero(arena, l->totalSize); \
	view.len = l->len; \
	i64 i = 0; \
	BASE_PTR_LIST_FOREACH(NODENAME, node, l) \
	{ \
		ELEM *elem = view.data; \
		elem[i] = node->val; \
		i++; \
	} \
	return view;\
} \

#define BASE_CREATE_LL_JUST_NODE_DEFS_EX(NAME, NODENAME, ELEM)

#define BASE_CREATE_LL_DEFS_EX(NAME, NODENAME, ELEM) \
BASE_CREATE_LL_JUST_NODE_DEFS_EX(NAME, NODENAME, ELEM) \
BASE_CREATE_LL_JUST_LIST_DEFS_EX(NAME, NODENAME, ELEM) \

#define BASE_CREATE_LL_DECLS(NAME, ELEM)   BASE_CREATE_LL_DECLS_EX(NAME, NAME##Node, ELEM)
#define BASE_CREATE_LL_DEFS(NAME, ELEM)   BASE_CREATE_LL_DEFS_EX(NAME, NAME##Node, ELEM)
#define BASE_CREATE_LL_DECLS_DEFS(NAME, ELEM)   \
BASE_CREATE_LL_DECLS_EX(NAME, NAME##Node, ELEM) \
BASE_CREATE_LL_DEFS_EX(NAME, NAME##Node, ELEM) \

#define BASE_CREATE_LL_JUST_LIST_DECLS_DEFS(NAME, ELEM)   \
BASE_CREATE_LL_JUST_LIST_DECLS_EX(NAME, NAME##Node, ELEM) \
BASE_CREATE_LL_JUST_LIST_DEFS_EX(NAME, NAME##Node, ELEM) \

// NODENAME is the node and value struct itself
#define BASE_CREATE_EFFICIENT_LL_DECLS(NAME, NODENAME) \
typedef struct NAME \
{ \
    struct NODENAME *first; \
    struct NODENAME *last; \
	u64 len; \
}NAME; \
typedef bool (*NAME##FirstFunc)(NODENAME *node); \
typedef bool (*NAME##FirstCaptureFunc)(NODENAME *node, void *capture); \
inline NODENAME *NAME##First(NAME *l, NAME##FirstFunc func); \
inline NODENAME *NAME##FirstCapture(NAME *l, void *capture, NAME##FirstCaptureFunc func); \
typedef bool (*NAME##WhereFunc)(NODENAME *node); \
inline ArrayView NAME##WhereArray(Arena *arena, NAME *l, NAME##WhereFunc func); \
inline void NAME##PushNodeLast(NAME *l, NODENAME *node); \
inline void NAME##PushNodeFirst(NAME *l, NODENAME *node); \
inline void NAME##InsertNode(NAME *l, NODENAME *prev, NODENAME *node); \
inline struct NODENAME *NAME##PopNodeLast(NAME *l);

#define BASE_CREATE_EFFICIENT_LL_DEFS(NAME, NODENAME) \
inline NODENAME *NAME##First(NAME *l, NAME##FirstFunc func) \
{ \
	BASE_PTR_LIST_FOREACH(NODENAME, node, l) \
	{ \
		if (func(node)) \
		{ \
			return node;\
		} \
	} \
	return null; \
} \
inline NODENAME *NAME##FirstCapture(NAME *l, void *capture, NAME##FirstCaptureFunc func) \
{ \
	BASE_PTR_LIST_FOREACH(NODENAME, node, l) \
	{ \
		if (func(node, capture)) \
		{ \
			return node;\
		} \
	} \
	return null; \
} \
inline ArrayView NAME##WhereArray(Arena *arena, NAME *l, NAME##WhereFunc func)\
{ \
    ArrayView view = {0};\
    ArrayView viewTemp = {0};\
    ArenaTemp temp = baseTempBegin(&arena, 1); \
    viewTemp.data = arenaPushArray(temp.arena, NODENAME*, l->len);\
    BASE_PTR_LIST_FOREACH(NODENAME, node, l)\
    {\
        if (func(node))\
        {\
            ((NODENAME**) (viewTemp.data))[viewTemp.len++] = node;\
        }\
    }\
    if (BASE_ANY(viewTemp))\
    {\
        view.data = arenaPushArray(arena, NODENAME*, viewTemp.len);\
        BASE_MEMCPY(view.data, viewTemp.data, viewTemp.len * sizeof(NODENAME*));\
        view.len = viewTemp.len;\
        view.elemSize = sizeof(NODENAME*);\
    }\
    baseTempEnd(temp);\
    return view;\
}\
inline void NAME##PushNodeLast(NAME *l, struct NODENAME *node) \
{ \
	BasePtrListNodePushLast(l, node); \
} \
inline void NAME##PushNodeFirst(NAME *l, struct NODENAME *node) \
{ \
	BasePtrListNodePushFirst(l, node); \
} \
inline void NAME##InsertNode(NAME *l, struct NODENAME *prev, struct NODENAME *node) \
{ \
	BasePtrListNodeInsert(l, prev, node); \
} \
inline struct NODENAME *NAME##PopNodeLast(NAME *l) \
{ \
	if (BASE_ANY_PTR(l)) \
	{ \
		l->len--; \
		struct NODENAME *last = l->last; \
	    BasePtrListNodeRemove(l, l->last); \
		return last; \
	} \
	return null; \
} \

#define BASE_CREATE_ARRAY_VIEW_DECLS_EX(NAME, ELEM)   \
typedef struct NAME NAME; \

#define BASE_CREATE_ARRAY_VIEW_DEFS_EX(NAME, ELEM)   \
typedef struct NAME \
{ \
	ELEM *data; \
	u64 len; \
}NAME;\


#define BASE_CREATE_ARRAY_VIEW_DECLS(NAME, ELEM)	BASE_CREATE_ARRAY_VIEW_DECLS_EX(NAME, ELEM)
#define BASE_CREATE_ARRAY_VIEW_DEFS(NAME, ELEM)	BASE_CREATE_ARRAY_VIEW_DEFS_EX(NAME, ELEM)
#define BASE_CREATE_ARRAY_VIEW_DECLS_DEFS(NAME, ELEM)	\
BASE_CREATE_ARRAY_VIEW_DECLS(NAME, ELEM) \
BASE_CREATE_ARRAY_VIEW_DEFS(NAME, ELEM)

#define ARRAY_VIEW_LIT_FROM_SIZED(T, ARRAY)		((T){ARRAY, .len = BASE_ARRAY_SIZE(ARRAY)})
#define ARRAY_VIEW_LIT(T, ARRAY, SIZE)			((T){ARRAY, .len = SIZE})

BASE_CREATE_ARRAY_VIEW_DECLS_DEFS(U8Array, u8)


#define BASE_U8CHUNKLIST_DEFAULT_CAP	128
typedef struct U8ChunkListNode
{
	struct U8ChunkListNode *next;
	struct U8ChunkListNode *prev;

	U8Array chunk;
	u64 cap;
}U8ChunkListNode;

typedef struct U8ChunkList
{
	U8ChunkListNode *first;
	U8ChunkListNode *last;
	
	u64 len;
	u64 totalLen;

	u64 defaultCap;
}U8ChunkList;

// disable this dumb warning
// #pragma warning( push )
// #pragma warning( disable : 4115)

typedef struct Arena Arena;
void U8ChunkListPushLast(struct Arena *arena, U8ChunkList *l, u8 n);
void U8ChunkListPushStr8Last(struct Arena *arena, U8ChunkList *l, str8 str);
U8Array U8ChunkListFlattenToArray(struct Arena *arena, U8ChunkList *l);
BASE_CREATE_LL_DECLS(U8ArrayList, U8Array)
// #pragma warning( pop ) 


// program entry related
typedef struct Str8List Str8List;
typedef void(*ProgramMainFunc)(Str8List *);

void BaseMainThreadEntry(ProgramMainFunc programMain, i64 argc, char **argv);

i64 baseColFprintf(FILE *fp, const char *fmt, ...);

i8 baseCharHexDigitToU8(u8 ch);
i8 baseCharBinDigitToU8(u8 ch);
i8 baseCharDigitToU8(u8 ch);

u8* baseMemcpyBigEndian(void *dst, void* src, u64 size);
i16 baseConvertToBigEndianI16(i16 num);
i16 baseConvertToLittleEndianI16(i16 num);
i32 baseConvertToBigEndianI32(i32 num);
i32 baseConvertToLittleEndianI32(i32 num);

u16 baseConvertToBigEndianU16(u16 num);
u16 baseConvertToLittleEndianU16(u16 num);
u32 baseConvertToBigEndianU32(u32 num);
u32 baseConvertToLittleEndianU32(u32 num);

#endif