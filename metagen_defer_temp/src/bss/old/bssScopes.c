#include "base/baseHash.h"
#include "base/baseStrings.h"

#include "bssScopes.h"

BASE_CREATE_EFFICIENT_LL_DEFS(BssValueList, BssValue);
BASE_CREATE_EFFICIENT_LL_DEFS(BssValueObjMembList, BssValueObjMemb);

BssSymTable BssNewSymTable(Arena *arena)
{
    const u64 slotCount = 13;

    BssSymTable symtable = {0};
    symtable.slots.data = arenaPushArray(arena, BssSymTableSlot, slotCount);
    symtable.slots.len = slotCount;

    return symtable;
}

BssScope *bssNewScope(Arena *arena, BssScope *parent)
{
    BssScope *scope = arenaPushType(arena, BssScope);
    scope->parent = parent;
    scope->table = BssNewSymTable(arena);

    return scope;
}

u64 bssScopeCalculateInsertIndex(BssScope *bssScope, str8 s)
{
    u64 hash = baseHashDJB2(s.data, s.len);
    u64 index = hash % bssScope->table.slots.len;

    return index;
}

BssSymTableSlotEntry *bssScopeAddEntry(BssScope *bssScope, BssSymTableSlotEntry *entry)
{
    u64 index = bssScopeCalculateInsertIndex(bssScope, entry->name);
    BaseListNodePushFirstEx(bssScope->table.slots.data[index], entry, hashPrev, hashNext);
    BaseListNodePushLastEx(bssScope->table, entry, prev, next);

    entry->ownerBssScope = bssScope;

    return entry;
}

BssSymTableSlotEntry *bssScopeFindEntryFromName(BssScope *bssScope, bool checkParent, str8 name)
{
    u64 index = bssScopeCalculateInsertIndex(bssScope, name);

    BssSymTableSlotEntry *found = null;

    BASE_LIST_FOREACH_EX(BssSymTableSlotEntry, entry, bssScope->table.slots.data[index], hashNext)
    {
        if(Str8Equals(name, entry->name, 0))
        {
            found = entry;
            break;
        }
    }

    if (found == null && ((bssScope->parent != null) && checkParent))
    {
        found = bssScopeFindEntryFromName(bssScope->parent, checkParent, name);
    }

    return found;
}