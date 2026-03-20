#ifndef BSS_SCOPE_H
#define BSS_SCOPE_H

#include "base/baseCore.h"
#include "bssCore.h"

typedef struct BssSymTableSlotEntry
{
    struct BssSymTableSlotEntry *next;
    struct BssSymTableSlotEntry *prev;

    str8 name;
    BssValue *value;
    struct BssScope *scopeDefinedIn;
}BssSymTableSlotEntry;

BASE_CREATE_EFFICIENT_LL_DECLS(BssSymTableSlotEntryList, BssSymTableSlotEntry)

typedef struct BssSymTable
{
    BssSymTableSlotEntryList entries;
}BssSymTable;

typedef struct BssScope
{
    struct BssScope *parent;

    // arena valid for the entire scope, this should be released when the scope ends
    // every allocation in this scope, eg for expr, stmts, etc, should be done from this aerna
    // except for values that should be bubbled up, they should be allocated on the parents scopearena
    Arena *scopeArena;

    bool isScopeInFunction;
    bool isReturnedSignaled;
    BssSymTable symTable;
}BssScope;

#define BSS_SCOPE_ZERO (&gBssScopeEmpty)
#define BSS_SYMTABLE_SLOT_ENTRY_ZERO (&gBsSymTableSlotEntryEmpty)

global BssScope gBssScopeEmpty;
global BssSymTableSlotEntry gBsSymTableSlotEntryEmpty;

BssScope *bssAllocScope(Arena *scopesArena, BssScope *parent, bool isScopeInFunction);
BssSymTableSlotEntry *bssScopeFindEntry(BssScope *scope, str8 name);
bool bssScopePushEntry(BssScope *scope, str8 name, BssSymTableSlotEntry **out);

#endif