#ifndef JSON_CORE_H
#define JSON_CORE_H

#include "base/baseCore.h"
#include "base/baseStrings.h"
#include "base/baseMemory.h"
#include "base/baseThreads.h"

typedef enum JSONValueKind
{
    JSON_VALUE_BOOL,
    JSON_VALUE_NULL,
    JSON_VALUE_OBJ,
    JSON_VALUE_ARRAY,
    JSON_VALUE_NUMBER,
    JSON_VALUE_STRING,
}JSONValueKind;

BASE_CREATE_LL_DECLS(JSONValueList, JSONValue)
BASE_CREATE_EFFICIENT_LL_DECLS(JSONObjMembList, JSONValue)

typedef struct JSONValue
{
    JSONValueKind kind;
    str8 strRep;
    union
    {
        bool asBool;
        JSONValueList asArray;
        f64 asNumber;
        str8 asStr8;
    };
}JSONValue;

typedef struct JSONObjMemb
{
    str8 name;
    JSONValue value;

    struct JSONObjMemb *next;
    struct JSONObjMemb *prev;
}JSONObjMemb;


#endif