#include "osNet.h"

#if OS_WIN32
#include "win32/osNetWin32.c"
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

i64 OSNetSocketSendAll(OSHandle socketHandle, U8Array buf)
{
    i64 sentAmount = 0;
    while (true)
    {
        U8Array sendBuffer = {.data = buf.data + sentAmount, .len = buf.len - sentAmount};
        i64 amountSent = OSNetSocketSend(socketHandle, sendBuffer);

        sentAmount += amountSent;

        if ((amountSent <= 0) || ((u64)sentAmount == buf.len))
        {
            break;
        }

    }

    return sentAmount;
}
U8ChunkList OSNetSocketRecieveAll(Arena *arena, OSHandle socketHandle)
{
    U8ChunkList retList = {.defaultCap = OS_NET_RECIEVE_ALL_BUFFER_SIZE};

    OSNetPollData pollData = {.socketHandle = socketHandle, .requestFlags = OS_NET_POLL_REQ_RECIEVE_FLAG};
    OSNetPollDataList pollList = {0};

    BaseListNodePushLast(pollList, &pollData);

    while(true)
    {
        i64 count = OSNetSocketPollList(pollList, 50);
        if (count > 0)
        {
            if (pollData.readyFlags & OS_NET_POLL_READY_RECIEVE_FLAG)
            {
                U8Array buffer =
                {
                    .data = (u8[OS_NET_RECIEVE_ALL_BUFFER_SIZE]){0},
                    .len = OS_NET_RECIEVE_ALL_BUFFER_SIZE,
                };

                i64 readAmount = OSNetSocketRecieve(socketHandle, &buffer);
                if (readAmount > 0)
                {
                    U8ChunkListPushStr8Last(arena, &retList, baseStr8(buffer.data, readAmount));
                }
                else
                {
                    break;
                }
            }
            else
            {
                break;
            }
        }
        else
        {
            break;
        }
    }

    return retList;
}