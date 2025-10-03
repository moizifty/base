#include "base\baseMetagen.h"
#include "base\baseMetagenCommon.gen.h"

metagen_genprintstructmemb()

void basePrintStructMember(void *member, MetagenStructMemb memb)
{
    u64 count = memb.isArray ? memb.arrayLen : 1;
    u64 size = memb.size / count;

    basePrintf("%S: ", memb.name);
    if (memb.isArray)
    {
        basePrintf("[");
    }

    for (u64 i = 0; i < count; i++)
    {
        switch (memb.type)
        {
            case METAGEN_TYPE_u8: basePrintf("%u", *((u8*)(member) + i)); break;
            case METAGEN_TYPE_u16: basePrintf("%u", *((u16*)(member) + i)); break;
            case METAGEN_TYPE_u32: basePrintf("%u", *((u32*)(member) + i)); break;
            case METAGEN_TYPE_u64: basePrintf("%llu", *((u64*)(member) + i)); break;

            case METAGEN_TYPE_i8: basePrintf("%d", *((i8*)(member) + i)); break;
            case METAGEN_TYPE_i16: basePrintf("%d", *((i16*)(member) + i)); break;
            case METAGEN_TYPE_i32: basePrintf("%d", *((i32*)(member) + i)); break;
            case METAGEN_TYPE_i64: basePrintf("%lld", *((i64*)(member) + i)); break;

            case METAGEN_TYPE_f32: basePrintf("%f", *((f32*)(member) + i)); break;
            case METAGEN_TYPE_f64: basePrintf("%f",*((f64*)(member) + i)); break;
            case METAGEN_TYPE_str8: basePrintf("%S", *((str8*)(member) + i)); break;
            case METAGEN_TYPE_bool: basePrintf("%d", *((bool*)(member) + i)); break;

            METAGEN_PRINT_MEMB_CUSTOM
        }
        
        if (i < count - 1)
        {
            basePrintf(", ");
        }
    }
    
    if (memb.isArray)
    {
        basePrintf("]");
    }
}
void basePrintStructEx(void *data, MetagenStructMembArray membs)
{
    basePrintf("{{");
    u8 *dataBuffer = (u8*)data;
    for (u64 i = 0; i < membs.len; i++)
    {
        MetagenStructMemb memb = membs.data[i];
        
        basePrintStructMember(dataBuffer + memb.offset, memb);

        if (i < membs.len - 1)
        {
            basePrintf(", ");
        }
    }
    basePrintf("}");
}