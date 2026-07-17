#include "jsonCore.h"

BASE_CREATE_EFFICIENT_LL_DEFS(JSONObjMembList, JSONObjMemb)
BASE_CREATE_LL_JUST_NODE_DEFS_EX(JSONValueList, JSONValueListNode, JSONValue)
BASE_CREATE_LL_JUST_LIST_DEFS_EX(JSONValueList, JSONValueListNode, JSONValue)

JSONValue *jsonFind(JSONValue *root, str8 path)
{
    ArenaTemp temp = baseTempBegin(null, 0);
    {
        Str8List pathSplit = Str8Split(temp.arena, path, STR8("."), 0, STR_SPLITFLAGS_DISCARD_EMPTY);

        BASE_LIST_FOREACH(Str8ListNode, pathNode, pathSplit)
        {
            u64 subscript = 0;
            if (U64TryFromStr8(pathNode->val, &subscript) && root->kind == JSON_VALUE_ARRAY)
            {
                if (subscript < root->asArray.len)
                {
                    u64 i = 0;
                    BASE_LIST_FOREACH_INDEX(JSONValueListNode, valNode, root->asArray, i)
                    {
                        if (i == subscript)
                        {
                            root = &valNode->val;
                            break;
                        }
                    }
                }
                else
                {
                    baseTempEnd(temp);
                    return null;
                }
            }
            else if (root->kind == JSON_VALUE_OBJ)
            {
                bool found = false;
                
                BASE_LIST_FOREACH(JSONObjMemb, memb, root->asObj)
                {
                    if (Str8Equals(memb->name, pathNode->val, 0))
                    {
                        root = &memb->value;
                        found = true;
                        break;
                    }
                }

                if (!found)
                {
                    baseTempEnd(temp);
                    return null;
                }
            }
            else
            {
                baseTempEnd(temp);
                return null;
            }
        }
    }

    baseTempEnd(temp);

    return root;
}

f64 jsonFindNumber(JSONValue *root, str8 path, f64 def)
{
    JSONValue *val = jsonFind(root, path);

    if (val != null && val->kind == JSON_VALUE_NUMBER)
    {
        return val->asNumber;
    }

    return def;
}

str8 jsonFindStr8(JSONValue *root, str8 path, str8 def)
{
    JSONValue *val = jsonFind(root, path);

    if (val != null && val->kind == JSON_VALUE_STRING)
    {
        return val->asStr8;
    }

    return def;
}

bool jsonFindBool(JSONValue *root, str8 path, bool def)
{
    JSONValue *val = jsonFind(root, path);

    if (val != null && val->kind == JSON_VALUE_BOOL)
    {
        return val->asBool;
    }

    return def;
}

JSONValue *jsonArrayIndex(JSONValue *array, u64 index)
{
    if (array->kind == JSON_VALUE_ARRAY && index < array->asArray.len)
    {
        u64 i = 0;
        BASE_LIST_FOREACH_INDEX(JSONValueListNode, val, array->asArray, i)
        {
            if (i == index)
            {
                return &val->val;
            }
        }
    }

    return null;
}