#include "base/baseMetagen.h"
#include "base/baseMetagenCommon.gen.h"

metagen_genprintstructmemb()

void StructMemberToStr8(Arena *arena, Str8List *list, void *member, MetagenStructMemb memb)
{
    u64 count = memb.isArray ? memb.arrayLen : 1;
    u64 size = memb.size / count;

    Str8ListPushLastFmt(arena, list, "%S: ", memb.name);
    if (memb.isArray)
    {
        Str8ListPushLastFmt(arena, list, "[");
    }

    for (u64 i = 0; i < count; i++)
    {
        switch (memb.type)
        {
            case METAGEN_TYPE_u8: Str8ListPushLastFmt(arena, list, "%u", *((u8*)(member) + i)); break;
            case METAGEN_TYPE_u16: Str8ListPushLastFmt(arena, list, "%u", *((u16*)(member) + i)); break;
            case METAGEN_TYPE_u32: Str8ListPushLastFmt(arena, list, "%u", *((u32*)(member) + i)); break;
            case METAGEN_TYPE_u64: Str8ListPushLastFmt(arena, list, "%llu", *((u64*)(member) + i)); break;

            case METAGEN_TYPE_i8: Str8ListPushLastFmt(arena, list, "%d", *((i8*)(member) + i)); break;
            case METAGEN_TYPE_i16: Str8ListPushLastFmt(arena, list, "%d", *((i16*)(member) + i)); break;
            case METAGEN_TYPE_i32: Str8ListPushLastFmt(arena, list, "%d", *((i32*)(member) + i)); break;
            case METAGEN_TYPE_i64: Str8ListPushLastFmt(arena, list, "%lld", *((i64*)(member) + i)); break;

            case METAGEN_TYPE_f32: Str8ListPushLastFmt(arena, list, "%f", *((f32*)(member) + i)); break;
            case METAGEN_TYPE_f64: Str8ListPushLastFmt(arena, list, "%f",*((f64*)(member) + i)); break;
            case METAGEN_TYPE_str8: Str8ListPushLastFmt(arena, list, "%S", *((str8*)(member) + i)); break;
            case METAGEN_TYPE_bool: Str8ListPushLastFmt(arena, list, "%d", *((bool*)(member) + i)); break;

            METAGEN_PRINT_MEMB_CUSTOM

            default:
            {
                basePrintf("Unhandled type in metagen printing.\n");
            }break;
        }
        
        if (i < count - 1)
        {
            Str8ListPushLastFmt(arena, list, ", ");
        }
    }
    
    if (memb.isArray)
    {
        Str8ListPushLastFmt(arena, list, "]");
    }
}
str8 StructToStr8(Arena *arena, Any any)
{
    Str8List list = {0};

    ArenaTemp temp = baseTempBegin(&arena, 1);

    Str8ListPushLastFmt(temp.arena, &list, "{");
    u8 *dataBuffer = (u8*)any.data;
    for (u64 i = 0; i < any.info.membs->len; i++)
    {
        MetagenStructMemb memb = any.info.membs->data[i];
        
        StructMemberToStr8(temp.arena, &list, dataBuffer + memb.offset, memb);

        if (i < any.info.membs->len - 1)
        {
            Str8ListPushLastFmt(temp.arena, &list, ", ");
        }
    }
    Str8ListPushLastFmt(temp.arena, &list, "}");

    str8 str = Str8ListJoin(arena, &list, null);
    baseTempEnd(temp);

    return str;
}