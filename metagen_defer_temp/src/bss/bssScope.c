#include "base/baseHash.h"
#include "bssScope.h"

readonly BssScope gBssScopeEmpty = {0};
readonly BssSymTableSlotEntry gBsSymTableSlotEntryEmpty = {0};

BASE_CREATE_EFFICIENT_LL_DEFS(BssSymTableSlotEntryList, BssSymTableSlotEntry)

BssScope *bssAllocScope(Arena *scopesArena, BssScope *parent, bool isScopeInFunction)
{
    BssScope *scope = arenaPushType(scopesArena, BssScope);
    scope->parent = parent;
    scope->isScopeInFunction = isScopeInFunction;
    scope->scopeArena = scopesArena;
    scope->isReturnedSignaled = false;

    return scope;
}

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
bool bssScopePushEntry(BssScope *scope, str8 name, BssSymTableSlotEntry **out)
{
    BssSymTableSlotEntry *existing = bssScopeFindEntry(scope, name);
    bool exists = existing != BSS_SYMTABLE_SLOT_ENTRY_ZERO;
    if (!exists)
    {
        existing = arenaPushType(scope->scopeArena, BssSymTableSlotEntry);
        existing->name = name;
        existing->scopeDefinedIn = scope;
        
        BssSymTableSlotEntryListPushNodeLast(&scope->symTable.entries, existing);
    }

    if (out != null) *out = existing;

    return exists;
}