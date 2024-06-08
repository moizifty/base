#ifndef BSS_TYPES_H
#define BSS_TYPES_H

#include "base\baseCore.h"
#include "bssLexer.h"

typedef enum BssTypeKind BssTypeKind;
typedef struct BssType BssType;
typedef struct BssTypeObjMemb BssTypeObjMemb;
typedef struct BssTypeFuncParam BssTypeFuncParam;
typedef struct BssTypeTable BssTypeTable;
typedef struct BssTypeList BssTypeList;
typedef struct BssTypeObjMembList BssTypeObjMembList;
typedef struct BssTypeFuncParamList BssTypeFuncParamList;

typedef enum BssTypeKind
{
    BSS_TYPE_INT,
    BSS_TYPE_BOOL,
    BSS_TYPE_STRING,
    BSS_TYPE_FUNC,
    BSS_TYPE_ARRAY,
    BSS_TYPE_OBJ,
}BssTypeKind;

typedef struct BssType
{
    struct BssType *next;
    struct BssType *prev;

    BssTypeKind kind;

    union
    {
        struct
        {
            BssType *ret;
            BssTypeFuncParamList *params;
        }func;

        struct
        {
            BssType *base;
        }array;

        struct
        {
            BssTypeObjMembList *membs;
        }obj;
    };
}BssType;

typedef struct BssTypeObjMemb
{
    struct BssTypeObjMemb *next;
    struct BssTypeObjMemb *prev;
    
    BssTok name;
    BssType *type;

    i64 index;
}BssTypeObjMemb;

typedef struct BssTypeFuncParam
{
    struct BssTypeFuncParam *next;
    struct BssTypeFuncParam *prev;

    BssTok name;
    BssType *type;
}BssTypeFuncParam;

BASE_CREATE_EFFICIENT_LL_DECLS(BssTypeList, BssType);

typedef struct BssTypeTable
{
    BssTypeList entries;
}BssTypeTable;

BASE_CREATE_EFFICIENT_LL_DECLS(BssTypeObjMembList, BssTypeObjMemb);
BASE_CREATE_EFFICIENT_LL_DECLS(BssTypeFuncParamList, BssTypeFuncParam);

BssType *bssAllocType(BaseArena *arena, BssTypeKind kind);
BssType *bssAllocTypeInt(BaseArena *arena, BssTypeTable *typeTable);
BssType *bssAllocTypeBool(BaseArena *arena, BssTypeTable *typeTable);
BssType *bssAllocTypeString(BaseArena *arena, BssTypeTable *typeTable);
BssType *bssAllocTypeFunc(BaseArena *arena, BssTypeTable *typeTable, BssType *ret, BssTypeFuncParamList *params);
BssType *bssAllocTypeArray(BaseArena *arena, BssTypeTable *typeTable, BssType *base);
BssType *bssAllocTypeObj(BaseArena *arena, BssTypeTable *typeTable, BssTypeObjMembList *membs);

BssTypeObjMemb *bssAllocTypeObjMemb(BaseArena *arena, BssTok name, BssType *type, i64 index);

bool bssAreBssTypesEqual(BssType *a, BssType *b);
bool bssIsTypeArray(BssType *a);
bool bssIsTypeInt(BssType *a);
bool bssIsTypeString(BssType *a);
bool bssIsTypeObj(BssType *a);
bool bssIsTypeFunc(BssType *a);

BssTypeFuncParam *bssTypeFuncHasParam(BssType *a, char *name);

BssTypeObjMemb *bssTypeObjHasMemb(BssType *a, char *name);

str8 bssTypeToString(BssType *type);

BssType *bssTypeTableAddEntry(BaseArena *arena, BssTypeTable *typeTable, BssType *type);
BssType *bssTypeTableTypeExists(BssTypeTable *typeTable, BssType *type);


#endif