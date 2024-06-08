#ifndef BSS_SCOPES_H
#define BSS_SCOPES_H

#include "base\baseCore.h"
#include "bssTypes.h"

typedef struct BssValue BssValue;
typedef struct BssValueObjMemb BssValueObjMemb;

BASE_CREATE_EFFICIENT_LL_DECLS(BssValueList, BssValue);
BASE_CREATE_EFFICIENT_LL_DECLS(BssValueObjMembList, BssValueObjMemb);

typedef struct BssValueObjMemb
{
    struct BssValueObjMemb *next;
    struct BssValueObjMemb *prev;

    str8 name;
    BssValue *val;
}BssValueObjMemb;

typedef struct BssValue
{
    struct BssValue *next;
    struct BssValue *prev;

    BssType *type;
    str8 strRep;

    bool hasBssValue;
    union
    {
        struct
        {
            i64 val;
        }integer;

        struct
        {
            bool val;
        }boolean;

        struct
        {
            Str8List list; // store a list, so its easier to concat and modify
        }str;

        struct
        {
            BssValueList elems;
        }arr;

        struct
        {
            BssValueObjMembList membs;
        }obj;
    };
}BssValue;

typedef struct BssSymTableSlotEntry
{
    struct BssSymTableSlotEntry *next;
    struct BssSymTableSlotEntry *prev;

    struct BssScope *ownerBssScope;
    str8 name;
    BssValue *value;
}BssSymTableSlotEntry;

BASE_CREATE_EFFICIENT_LL_DECLS(BssSymTableSlot, BssSymTableSlotEntry);
BASE_CREATE_ARRAY_VIEW_DECLS_DEFS(BssSymTableSlotArray, BssSymTableSlot);

typedef struct BssSymTable
{
    BssSymTableSlotArray slots;
}BssSymTable;

typedef struct BssScope
{
    struct BssScope *parent;

    struct BssScope *next;
    struct BssScope *prev;

    BssSymTable table;
}BssScope;

BssSymTable BssNewSymTable(BaseArena *arena);

BssScope *bssNewScope(BaseArena *arena, BssScope *parent);

u64 bssScopeCalculateInsertIndex(BssScope *bssScope, str8 s);
BssSymTableSlotEntry *bssScopeAddEntry(BssScope *bssScope, BssSymTableSlotEntry *entry);
BssSymTableSlotEntry *bssScopeFindEntryFromName(BssScope *bssScope, bool checkParent, str8 name);

#endif