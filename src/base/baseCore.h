#ifndef BASE_CORE_H
#define BASE_CORE_H

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

// Bitwise
#define BASE_SET_FLAG(n, f)    ((n) |= (f));

// memory
#define BASE_MEMCPY memcpy
#define BASE_MEMSET memset
#define BASE_MEMZERO(DEST, SZ) BASE_MEMSET((DEST), 0, (SZ))

// lists, and stuff
#define CheckNull(p) ((p)==0)
#define SetNull(p) ((p)=0)

#define BaseDllNodeInsertEx(f, l, p, n, prev, next)  \
((CheckNull(f)) ? ((f) = (l) = (n), (n)->prev = (n)->next = null) \
: ((CheckNull(p)) ? ((n)->next = (f), (n)->prev = null, (f)->prev = (n), (f) = (n)) \
: ((!CheckNull((p)->next)) ? (p)->next->prev = (n) : 0), ((n)->next = (p)->next, (n)->prev = (p), (p)->next = (n), (((p) == (l)) ? (l) = (n) : 0))))

#define BaseDllNodeInsert(f, l, p, n)   BaseDllNodeInsertEx(f, l, p, n, prev, next)
#define BaseDllNodePushLast(f, l, n)    BaseDllNodeInsertEx(f, l, l, n, prev, next)
#define BaseDllNodePushFirst(f, l, n)   BaseDllNodeInsertEx(l, f, f, n, next, prev)

#define BASE_CREATE_LL_DECLS_EX(NAME, NODENAME, ELEM) \
typedef struct NODENAME NODENAME; \
typedef struct NAME NAME; \
void NAME##PushNodeLast(NAME *l, NODENAME *node); \
void NAME##PushNodeFirst(NAME *l, NODENAME *node); \
void NAME##InsertNode(NAME *l, NODENAME *prev, NODENAME *node); \
void NAME##PushLast(BaseArena *arena, NAME *l, ELEM value); \
void NAME##PushFirst(BaseArena *arena, NAME *l, ELEM value); \
void NAME##PushInsert(BaseArena *arena, NAME *l, NODENAME *prev, ELEM value);

#define BASE_CREATE_LL_DEFS_EX(NAME, NODENAME, ELEM, TOTALSIZEEXPR) \
typedef struct NODENAME NODENAME; \
typedef struct NAME NAME; \
typedef struct NODENAME \
{ \
	NODENAME *next; \
	NODENAME *prev; \
	ELEM val; \
}NODENAME; \
 \
typedef struct NAME \
{ \
	NODENAME *first; \
	NODENAME *last; \
	u64 len; \
	u64 totalSize; \
}NAME; \
void NAME##PushNodeLast(NAME *l, NODENAME *node) \
{ \
	BaseDllNodePushLast(l->first, l->last, node); \
	l->len += 1; \
	l->totalSize += TOTALSIZEEXPR; \
} \
void NAME##PushNodeFirst(NAME *l, NODENAME *node) \
{ \
	BaseDllNodePushFirst(l->first, l->last, node); \
	l->len += 1; \
	l->totalSize += TOTALSIZEEXPR; \
} \
void NAME##InsertNode(NAME *l, NODENAME *prev, NODENAME *node) \
{ \
	BaseDllNodeInsert(l->first, l->last, prev, node); \
	l->len += 1; \
	l->totalSize += TOTALSIZEEXPR; \
} \
void NAME##PushLast(BaseArena *arena, NAME *l, ELEM value) \
{ \
	NODENAME *n = baseArenaPush(arena, sizeof(NODENAME)); \
	n->val = value; \
	NAME##PushNodeLast(l, n); \
} \
void NAME##PushFirst(BaseArena *arena, NAME *l, ELEM value) \
{ \
	NODENAME *n = baseArenaPush(arena, sizeof(NODENAME)); \
	n->val = value; \
	NAME##PushNodeFirst(l, n); \
} \
void NAME##PushInsert(BaseArena *arena, NAME *l, NODENAME *prev, ELEM value) \
{ \
	NODENAME *n = baseArenaPush(arena, sizeof(NODENAME)); \
	n->val = value; \
	NAME##InsertNode(l, prev, n); \
} \

#define BASE_CREATE_LL_DECLS(NAME, ELEM)   BASE_CREATE_LL_DECLS_EX(NAME, NAME##Node, ELEM)
#define BASE_CREATE_LL_DEFS(NAME, ELEM)   BASE_CREATE_LL_DEFS_EX(NAME, NAME##Node, ELEM, sizeof(node->val))
#define BASE_CREATE_LL_DECLS_DEFS(NAME, ELEM)   \
BASE_CREATE_LL_DECLS(NAME, ELEM) \
BASE_CREATE_LL_DEFS(NAME, ELEM) \


// program entry related
typedef void(*ProgramMainFunc)(void);

void BaseMainThreadEntry(ProgramMainFunc programMain, u64 argc, u8 **argv);
#endif