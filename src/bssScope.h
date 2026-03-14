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
}BssSymTableSlotEntry;

BASE_CREATE_EFFICIENT_LL_DECLS(BssSymTableSlotEntryList, BssSymTableSlotEntry)

typedef struct BssSymTable
{
    BssSymTableSlotEntryList entries;
}BssSymTable;

typedef struct BssScope
{
    struct BssScope *parent;

    BssSymTable symTable;
}BssScope;

#define BSS_SYMTABLE_SLOT_ENTRY_ZERO (&gBsSymTableSlotEntryEmpty)

global BssSymTableSlotEntry gBsSymTableSlotEntryEmpty;

BssSymTableSlotEntry *bssScopeFindEntry(BssScope *scope, str8 name);
bool bssScopePushEntry(BssInterp *interp, BssScope *scope, str8 name, BssSymTableSlotEntry **out);

#endif