#ifndef BASE_METAGEN_H
#define BASE_METAGEN_H

#include "base/baseCore.h"

// metagen commands
#define metagen_gentable(...)
#define metagen_introspect(...)
#define metagen_introspectexclude(...)
#define metagen_introspectnote(...)
#define metagen_genprintstructmemb(...)
#define metagen_embedfile(name, path, mode)
#define metagen_defer

#define basePrintStruct(T, S) basePrintStructEx(&(S), (g##T##StructInfo))
#define ANY(T, S)  ((Any){.data = &(S), .info = g##T##StructInfo})

typedef enum MetagenTypeKind
{
    METAGEN_TYPE_u8,
    METAGEN_TYPE_u16,
    METAGEN_TYPE_u32,
    METAGEN_TYPE_u64,
    METAGEN_TYPE_i8,
    METAGEN_TYPE_i16,
    METAGEN_TYPE_i32,
    METAGEN_TYPE_i64,
    METAGEN_TYPE_f32,
    METAGEN_TYPE_f64,
    METAGEN_TYPE_str8,
    METAGEN_TYPE_str16,
    METAGEN_TYPE_bool,
    METAGEN_TYPE_void,
    METAGEN_TYPE_CUSTOM_BEGIN
}MetagenTypeKind;

typedef struct MetagenStructMemb
{
    str8 name;
    MetagenTypeKind type;

    u64 size;
    u64 offset;
    u64 isPointer : 1;
    u64 isArray : 1;

    u64 arrayLen;
    str8 note;
}MetagenStructMemb;

BASE_CREATE_ARRAY_VIEW_DECLS_DEFS(MetagenStructMembArray, MetagenStructMemb)

typedef struct MetagenStruct
{
    str8 name;
    MetagenTypeKind typeid;

    u64 size;
    u64 align;
    MetagenStructMembArray *membs;
}MetagenStruct;

typedef struct Any
{
    void *data;
    MetagenStruct info;
}Any;

typedef struct Str8List Str8List;
str8 StructToStr8(Arena *arena, Any any);
#include "base/baseMetagenCommon.gen.h"

#endif