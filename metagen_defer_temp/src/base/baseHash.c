#include "baseHash.h"

u64 baseHashDJB2(u8 *bytes, u64 len)
{
    u64 hash = 5381;

    for(u64 i = 0; i < len; i++)
    {
        hash = ((hash << 5) + hash) + bytes[i]; /* hash * 33 + c */
    }

    return hash;
}