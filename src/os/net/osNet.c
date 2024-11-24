#include "osNet.h"

#ifdef OS_WIN32
#include "win32\osNetWin32.c"
#else
#error Platform not defined
#endif

BASE_CREATE_EFFICIENT_LL_DEFS(OSNetAddrInfoList, OSNetAddrInfo);
BASE_CREATE_EFFICIENT_LL_DEFS(OSNetAddrList, OSNetAddr);

BASE_CREATE_EFFICIENT_LL_DEFS(OSNetPollDataList, OSNetPollData);

u16 OSNetChecksum(u16 *data, u64 len)
{
    if (len == 0)
    {
        return 0;
    }

    u32 sum = 0;
    while (len > 0)
    {
        sum += *(data++);

        len -= 2;
    }

    while (sum >> 16)
    {
        u16 overflow = (u16)(sum >> 16);
        sum = (sum & 0xffff) + overflow;
    }

    return (u16)~sum;
}