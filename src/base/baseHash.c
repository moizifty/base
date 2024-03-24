#include "baseHash.h"

u64 hashDJB2(i8 *bytes, i64 len)
{
    u64 hash = 5381;

    for(i64 i = 0; i < len; i++)
    {
        hash = ((hash << 5) + hash) + bytes[i]; /* hash * 33 + c */
    }

    return hash;
}