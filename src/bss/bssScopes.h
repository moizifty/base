#ifndef BSS_SCOPES_H
#define BSS_SCOPES_H

#include "base\baseCore.h"
#include "bssTypes.h"

#define BSS_VALUE_TRYINT(v, i)    (((v) && (v)->hasBssValue && bssIsTypeInt((v)->type)) ? (((i) = (v)->integer.val), true) : (((i) = 0), false))
#define BSS_VALUE_TRYBOOL(v, b)    (((v) && (v)->hasBssValue && bssIsTypeBool((v)->type)) ? (((b) = (v)->boolean.val), true) : (((b) = false), false))
#define BSS_VALUE_TRYARRAY(v, a)    (((v) && (v)->hasBssValue && bssIsTypeArray((v)->type)) ? (((a) = (v)->arr.val), true) : (((a) = (BssValueList){0}), false))

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
            Str8List val; // store a list, so its easier to concat and modify
        }str;

        struct
        {
            BssValueList val;
        }arr;

        struct
        {
            BssValueObjMembList val;
        }obj;
    };
}BssValue;

typedef struct BssSymTableSlotEntry
{
    struct BssSymTableSlotEntry *hashNext;
    struct BssSymTableSlotEntry *hashPrev;

    struct BssSymTableSlotEntry *next;
    struct BssSymTableSlotEntry *prev;

    struct BssScope *ownerBssScope;
    str8 name;
    BssType *type;
    BssValue *value;
}BssSymTableSlotEntry;

BASE_CREATE_EFFICIENT_LL_DECLS(BssSymTableSlot, BssSymTableSlotEntry);
BASE_CREATE_ARRAY_VIEW_DECLS_DEFS(BssSymTableSlotArray, BssSymTableSlot);

typedef struct BssSymTable
{
    //stres entrys as dll and also ijn hashtable,
    //you sometimes want to go over entries in order of being added
    //hence why u want dll aswell hashlinks
    BssSymTableSlotEntry *first;
    BssSymTableSlotEntry *last;
    u64 len;

    BssSymTableSlotArray slots;
}BssSymTable;

typedef struct BssScope
{
    struct BssScope *parent;

    struct BssScope *next;
    struct BssScope *prev;

    BssSymTable table;
}BssScope;

BssSymTable BssNewSymTable(Arena *arena);

BssScope *bssNewScope(Arena *arena, BssScope *parent);

u64 bssScopeCalculateInsertIndex(BssScope *bssScope, str8 s);
BssSymTableSlotEntry *bssScopeAddEntry(BssScope *bssScope, BssSymTableSlotEntry *entry);
BssSymTableSlotEntry *bssScopeFindEntryFromName(BssScope *bssScope, bool checkParent, str8 name);

#endif