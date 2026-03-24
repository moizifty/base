#ifndef BSS_TYPES_H
#define BSS_TYPES_H

#include "base/baseCore.h"
#include "bssLexer.h"

typedef enum BssTypeKind BssTypeKind;
typedef struct BssType BssType;
typedef struct BssTypeObjMemb BssTypeObjMemb;
typedef struct BssTypeFuncParam BssTypeFuncParam;
typedef struct BssTypeTable BssTypeTable;
typedef struct BssTypeList BssTypeList;

typedef enum BssTypeKind
{
    BSS_TYPE_INVALID = 0,

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
            struct BssScope *scope;
        }func;

        struct
        {
            BssType *base;
        }array;

        struct
        {
            struct BssScope *membScope;
        }obj;
    };
}BssType;

BASE_CREATE_EFFICIENT_LL_DECLS(BssTypeList, BssType);

typedef struct BssTypeTable
{
    BssTypeList entries;
}BssTypeTable;

BssType *bssAllocType(Arena *arena, BssTypeKind kind);
BssType *bssAllocTypeInt(Arena *arena);
BssType *bssAllocTypeBool(Arena *arena);
BssType *bssAllocTypeString(Arena *arena);
BssType *bssAllocTypeFunc(Arena *arena, BssType *ret, struct BssScope *scope);
BssType *bssAllocTypeArray(Arena *arena, BssType *base);
BssType *bssAllocTypeObj(Arena *arena, struct BssScope *scope);

BssTypeObjMemb *bssAllocTypeObjMemb(Arena *arena, BssTok name, BssType *type, i64 index);

bool bssAreBssTypesEqual(BssType *a, BssType *b);
bool bssIsTypeArray(BssType *a);
bool bssIsTypeInt(BssType *a);
bool bssIsTypeBool(BssType *a);
bool bssIsTypeString(BssType *a);
bool bssIsTypeObj(BssType *a);
bool bssIsTypeFunc(BssType *a);

BssTypeFuncParam *bssTypeFuncHasParam(BssType *a, char *name);

BssTypeObjMemb *bssTypeObjHasMemb(BssType *a, char *name);

str8 bssTypeToString(BssType *type);

BssType *bssTypeTableAddEntry(Arena *arena, BssTypeTable *typeTable, BssType *type);
BssType *bssTypeTableTypeExists(BssTypeTable *typeTable, BssType *type);


#endif