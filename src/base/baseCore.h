#ifndef BASE_CORE_H
#define BASE_CORE_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "baseCoreTypes.h"

// General
#define BASE_BYTES(NUM) ((u64)(NUM))
#define BASE_KILOBYTES(NUM) ((u64)(BASE_BYTES(NUM)) * 1024u)
#define BASE_MEGABYTES(NUM) ((u64)(BASE_KILOBYTES(NUM)) * 1024u)
#define BASE_GIGABYTES(NUM) ((u64)(BASE_MEGABYTES(NUM)) * 1024u)

#define BASE_IS_POWER_OF_2(NUM) ((NUM) & ((NUM) - 1))
#define BASE_NUM_BETWEEN(X, S, E)     (((X) >= (S)) && ((X) <= (E)))
#define BASE_ARRAY_SIZE(ARR)  ((sizeof(ARR)) / (sizeof((ARR[0]))))
#define BASE_MAX(A, B)  (((A) > (B)) ? (A) : (B))

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

// memory
#define BASE_MEMCPY memcpy
#define BASE_MEMSET memset
#define BASE_MEMZERO(DEST, SZ) BASE_MEMSET((DEST), 0, (SZ))

// lists, and stuff
#define BASE_PTR_LIST_FOREACH(NODETYPE, NAME, LIST)		for(NODETYPE *NAME = (LIST)->first; (NAME) != null; (NAME) = (NAME)->next)
#define BASE_LIST_FOREACH(NODETYPE, NAME, LIST)			for(NODETYPE *NAME = (LIST).first; (NAME) != null; (NAME) = (NAME)->next)

#define BASE_PTR_LIST_FOREACH_INDEX(NODETYPE, NAME, LIST, INDEX)		for(NODETYPE *NAME = (LIST)->first; (NAME) != null; (NAME) = (NAME)->next, (INDEX)++)
#define BASE_LIST_FOREACH_INDEX(NODETYPE, NAME, LIST, INDEX)			for(NODETYPE *NAME = (LIST).first; (NAME) != null; (NAME) = (NAME)->next, (INDEX)++)

#define CheckNull(p) ((p)==0)
#define SetNull(p) ((p)=0)

#define BaseDllNodeInsertEx(f, l, p, n, prev, next)  \
((CheckNull(f)) ? ((f) = (l) = (n), (n)->prev = (n)->next = null) \
: ((CheckNull(p)) ? ((n)->next = (f), (n)->prev = null, (f)->prev = (n), (f) = (n)) \
: ((!CheckNull((p)->next)) ? (p)->next->prev = (n) : 0), ((n)->next = (p)->next, (n)->prev = (p), (p)->next = (n), (((p) == (l)) ? (l) = (n) : 0))))

#define BaseDllNodeInsert(f, l, p, n)   BaseDllNodeInsertEx(f, l, p, n, prev, next)
#define BaseDllNodePushLast(f, l, n)    BaseDllNodeInsertEx(f, l, l, n, prev, next)
#define BaseDllNodePushFirst(f, l, n)   BaseDllNodeInsertEx(l, f, f, n, next, prev)

#define BASE_ANY_PTR(pL)     (((pL) != NULL) && (pL)->len != 0)
#define BASE_ANY(L)     ((pL)->len != 0)

#define BASE_PTR_LL_FIRST(pLL, DEFVAL)     (BASE_ANY_PTR(pLL) ? (pLL)->first->val : DEFVAL)
#define BASE_LL_FIRST(LL, DEFVAL)          (BASE_ANY(pLB) ? (pLB).first->val : DEFVAL)

#define BASE_PTR_LL_LAST(pLL, DEFVAL)      (BASE_ANY_PTR(pLL) ? (pLL)->last->val : DEFVAL)
#define BASE_LL_LAST(LL, DEFVAL)           (BASE_ANY(pLB) ? (pLB).last->val : DEFVAL)

#define BASE_CREATE_LL_JUST_LIST_DECLS_EX(NAME, NODENAME, ELEM) \
typedef struct NAME NAME; \
typedef struct NODENAME NODENAME; \
inline void NAME##PushNodeLast(NAME *l, NODENAME *node); \
inline void NAME##PushNodeFirst(NAME *l, NODENAME *node); \
inline void NAME##InsertNode(NAME *l, NODENAME *prev, NODENAME *node); \
inline void NAME##PushLast(BaseArena *arena, NAME *l, ELEM value); \
inline void NAME##PushFirst(BaseArena *arena, NAME *l, ELEM value); \
inline void NAME##PushInsert(BaseArena *arena, NAME *l, NODENAME *prev, ELEM value); \

#define BASE_CREATE_LL_JUST_NODE_DECLS_EX(NAME, NODENAME, ELEM) \
typedef struct NAME NAME; \
typedef struct NODENAME NODENAME; \

#define BASE_CREATE_LL_DECLS_EX(NAME, NODENAME, ELEM) \
BASE_CREATE_LL_JUST_LIST_DECLS_EX(NAME, NODENAME, ELEM) \
BASE_CREATE_LL_JUST_NODE_DECLS_EX(NAME, NODENAME, ELEM)  \

#define BASE_CREATE_LL_JUST_LIST_DEFS_EX(NAME, NODENAME, ELEM) \
typedef struct NAME NAME; \
typedef struct NODENAME NODENAME; \
typedef struct NAME \
{ \
	NODENAME *first; \
	NODENAME *last; \
	u64 len; \
	u64 totalSize; \
}NAME; \
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
inline void NAME##PushLast(BaseArena *arena, NAME *l, ELEM value) \
{ \
	NODENAME *n = baseArenaPush(arena, sizeof(NODENAME)); \
	n->val = value; \
	NAME##PushNodeLast(l, n); \
} \
inline void NAME##PushFirst(BaseArena *arena, NAME *l, ELEM value) \
{ \
	NODENAME *n = baseArenaPush(arena, sizeof(NODENAME)); \
	n->val = value; \
	NAME##PushNodeFirst(l, n); \
} \
inline void NAME##PushInsert(BaseArena *arena, NAME *l, NODENAME *prev, ELEM value) \
{ \
	NODENAME *n = baseArenaPush(arena, sizeof(NODENAME)); \
	n->val = value; \
	NAME##InsertNode(l, prev, n); \
} \
inline ArrayView NAME##FlattenToArray(BaseArena *arena, NAME *l) \
{ \
	ArrayView view = {0}; \
	view.data = baseArenaPushNoZero(arena, l->totalSize); \
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

#define BASE_CREATE_LL_JUST_NODE_DEFS_EX(NAME, NODENAME, ELEM) \
typedef struct NAME NAME; \
typedef struct NODENAME NODENAME; \
typedef struct NODENAME \
{ \
	NODENAME *next; \
	NODENAME *prev; \
	ELEM val; \
}NODENAME;

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

#define ARRAY_VIEW_LIT_FROM_SIZED_EX(NAME, ARRAY)		((NAME){ARRAY, .len = BASE_ARRAY_SIZE(ARRAY)})
#define ARRAY_VIEW_LIT_EX(NAME, ARRAY, SIZE)			((NAME){ARRAY, .len = SIZE})

#define ARRAY_VIEW_LIT_FROM_SIZED(ARRAY)		((ArrayView){ARRAY, .len = BASE_ARRAY_SIZE(ARRAY)})
#define ARRAY_VIEW_LIT(ARRAY, SIZE)			((ArrayView){ARRAY, .len = SIZE})

// program entry related
typedef struct CmdLineHashMap CmdLineHashMap;
typedef void(*ProgramMainFunc)(CmdLineHashMap *);

void BaseMainThreadEntry(ProgramMainFunc programMain, i64 argc, i8 **argv);

i64 baseColFprintf(FILE *fp, const char *fmt, ...);

i64 baseHexDigitToInt(int ch);
i64 baseBinDigitToInt(int ch);
i64 baseCStyleIntLiteralToInt(str8 str);
#endif