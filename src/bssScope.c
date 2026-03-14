#include "base/baseHash.h"
#include "bssScope.h"

readonly BssSymTableSlotEntry gBsSymTableSlotEntryEmpty = {0};

BASE_CREATE_EFFICIENT_LL_DEFS(BssSymTableSlotEntryList, BssSymTableSlotEntry)

BssSymTableSlotEntry *bssScopeFindEntry(BssScope *scope, str8 name)
{
    BASE_LIST_FOREACH(BssSymTableSlotEntry, entry, scope->symTable.entries)
    {
        if (Str8Equals(name, entry->name, 0))
        {
            return entry;
        }
    }

    return scope->parent ? bssScopeFindEntry(scope->parent, name) : BSS_SYMTABLE_SLOT_ENTRY_ZERO;
}
bool bssScopePushEntry(BssInterp *interp, BssScope *scope, str8 name, BssSymTableSlotEntry **out)
{
    BssSymTableSlotEntry *existing = bssScopeFindEntry(scope, name);
    bool exists = existing != BSS_SYMTABLE_SLOT_ENTRY_ZERO;
    if (!exists)
    {
        existing = arenaPushType(interp->arena, BssSymTableSlotEntry);
        existing->name = name;

        BssSymTableSlotEntryListPushNodeLast(&scope->symTable.entries, existing);
    }

    if (out != null) *out = existing;

    return exists;
}