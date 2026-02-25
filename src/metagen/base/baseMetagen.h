#ifndef BASE_METAGEN_H
#define BASE_METAGEN_H

#include "base/baseCore.h"

// metagen commands
#define metagen_gentable(...)
#define metagen_introspect(...)
#define metagen_introspectexclude(...)
#define metagen_genprintstructmemb(...)
#define metagen_embedfile(name, path, mode)

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
}MetagenStructMemb;

BASE_CREATE_ARRAY_VIEW_DECLS_DEFS(MetagenStructMembArray, MetagenStructMemb);

void basePrintStruct(void *data, MetagenStructMembArray membs);

#include "base/baseMetagenCommon.gen.h"

#endif