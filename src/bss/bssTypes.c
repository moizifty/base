#include "bssTypes.h"
#include "bssScopes.h"

BssType *bssAllocType(BaseArena *arena, BssTypeKind kind)
{
    BssType *t = baseArenaPushType(arena, BssType);
    t->kind = kind;

    return t;
}
BssType *bssAllocTypeInt(BaseArena *arena)
{
    BssType *t = bssAllocType(arena, BSS_TYPE_INT);

    return t;
}
BssType *bssAllocTypeBool(BaseArena *arena)
{
    BssType *t = bssAllocType(arena, BSS_TYPE_BOOL);

    return t;
}
BssType *bssAllocTypeString(BaseArena *arena)
{
    BssType *t = bssAllocType(arena, BSS_TYPE_STRING);

    return t;
}
BssType *bssAllocTypeFunc(BaseArena *arena, BssType *ret, struct BssScope *scope)
{
    BssType *t = bssAllocType(arena, BSS_TYPE_FUNC);
    t->func.ret = ret;
    t->func.scope = scope;

    return t;
}
BssType *bssAllocTypeArray(BaseArena *arena, BssType *base)
{
    BssType *t = bssAllocType(arena, BSS_TYPE_ARRAY);
    t->array.base = base;

    return t;
}
BssType *bssAllocTypeObj(BaseArena *arena, struct BssScope *scope)
{
    BssType *t = bssAllocType(arena, BSS_TYPE_OBJ);
    t->obj.membScope = scope;

    return t;
}

bool bssAreBssTypesEqual(BssType *a, BssType *b)
{
    if(a->kind == b->kind)
    {
        if(!bssIsTypeObj(a) && !bssIsTypeFunc(a))
        {
            return true;
        }
    
        switch(a->kind)
        {
            case BSS_TYPE_OBJ:
            {
                if(a->obj.membScope->table.len == b->obj.membScope->table.len)
                {
                    BssSymTableSlotEntry *eB = b->obj.membScope->table.first;
                    BASE_LIST_FOREACH(BssSymTableSlotEntry, eA, a->obj.membScope->table)
                    {
                        if (!bssAreBssTypesEqual(eB->type, eA->type))
                        {
                            return false;
                        }
                    }

                    return true;
                }

            }break;

            default:
            {
                baseColEPrintf("Unhandled type is bssAreBsstypesequal.\n");
            }break;
        }
    }

    return false;
}

bool bssIsTypeArray(BssType *a)
{
    return a->kind == BSS_TYPE_ARRAY;
}
bool bssIsTypeInt(BssType *a)
{
    return a->kind == BSS_TYPE_INT;
}
bool bssIsTypeBool(BssType *a)
{
    return a->kind == BSS_TYPE_BOOL;
}
bool bssIsTypeString(BssType *a)
{
    return a->kind == BSS_TYPE_STRING;
}
bool bssIsTypeObj(BssType *a)
{
    return a->kind == BSS_TYPE_OBJ;
}
bool bssIsTypeFunc(BssType *a)
{
    return a->kind == BSS_TYPE_FUNC;
}