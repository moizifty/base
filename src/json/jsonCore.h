#ifndef JSON_CORE_H
#define JSON_CORE_H

#include "base/baseCore.h"
#include "base/baseStrings.h"
#include "base/baseMemory.h"
#include "base/baseThreads.h"

typedef enum JSONValueKind
{
    JSON_VALUE_INVALID,
    JSON_VALUE_BOOL,
    JSON_VALUE_NULL,
    JSON_VALUE_OBJ,
    JSON_VALUE_ARRAY,
    JSON_VALUE_NUMBER,
    JSON_VALUE_STRING,
}JSONValueKind;

typedef struct JSONValueList 
{ 
    struct JSONValueListNode *first; 
    struct JSONValueListNode *last; 
    u64 len; 
    u64 totalSize; 
}JSONValueList;

typedef struct JSONObjMemb JSONObjMemb;
BASE_CREATE_EFFICIENT_LL_DECLS(JSONObjMembList, JSONObjMemb)

typedef struct JSONValue
{
    JSONValueKind kind;
    str8 strRep;
    union
    {
        bool asBool;
        JSONValueList asArray;
        JSONObjMembList asObj;
        f64 asNumber;
        str8 asStr8;
    };
}JSONValue;

BASE_CREATE_LL_JUST_NODE_DECLS_EX(JSONValueList, JSONValueListNode, JSONValue)
BASE_CREATE_LL_JUST_LIST_FUNC_DECLS_EX(JSONValueList, JSONValueListNode, JSONValue)

typedef struct JSONObjMemb
{
    str8 name;
    JSONValue value;

    struct JSONObjMemb *next;
    struct JSONObjMemb *prev;
}JSONObjMemb;

// path can be example: obj.child.subchild.etc
// if its an array, you can do, obj.child.array.9
// or even obj.child.array.12.name.len.1
JSONValue *jsonFind(JSONValue *root, str8 path);

f64 jsonFindNumber(JSONValue *root, str8 path, f64 def);
str8 jsonFindStr8(JSONValue *root, str8 path, str8 def);
bool jsonFindBool(JSONValue *root, str8 path, bool def);

JSONValue *jsonArrayIndex(JSONValue *array, u64 index);
#endif