#ifndef OS_NET_H
#define OS_NET_H

#include "base\baseCore.h"
#include "base\baseStrings.h"
#include "base\baseLog.h"
#include "base\baseMath.h"
#include "base\baseThreads.h"
#include "base\baseMemory.h"
#include "os\core\osCore.h"

#ifdef OS_WIN32
#include "win32\osNetWin32.h"
#else
#error Platform not defined
#endif

#define OS_NET_RECIEVE_ALL_BUFFER_SIZE 256
#define OS_NET_SEND_ALL_BUFFER_SIZE 256

typedef enum OSNetAddrKind
{
    OS_NET_ADDR_EITHER,
    OS_NET_ADDR_IPV4,
    OS_NET_ADDR_IPV6,
}OSNetAddrKind;

typedef enum OSNetSocketKind
{
    OS_NET_SOCK_STREAM,
    OS_NET_SOCK_DATAGRAM,
    OS_NET_SOCK_RAW,
}OSNetSocketKind;

typedef enum OSNetProtocolKind
{
    OS_NET_PROTOCOL_EITHER,
    OS_NET_PROTOCOL_TCP,
    OS_NET_PROTOCOL_UDP,
    OS_NET_PROTOCOL_ICMP,
    OS_NET_PROTOCOL_RAW,
}OSNetProtocolKind;

/**
 * Think of socket options as a table, the level param to setsockopt, tells you what table the setting to set on
 * (each setting might be on a different table related to its own section)
 * optname param tells you the options name 
 */
typedef enum OSNetSocketOptionLevel
{
    OS_NET_SOCK_OPTION_IPV4,
    OS_NET_SOCK_OPTION_TCP,
    OS_NET_SOCK_OPTION_SOCKET,
    // THERES MORE but only this is needed rn for my needs
}OSNetSocketOptionLevel;

typedef enum OSNetSocketOptionName
{
    OS_NET_SOCK_OPTION_IPV4__IP_HEADER_INCLUDE, // IP_HDRINCL
    OS_NET_SOCK_OPTION_TCP__MAX_SEGMENT_SIZE, // TCP_MAXSEG
    OS_NET_SOCK_OPTION_SOCKET__MAX_SEND_BUFFER_SIZE, // SO_SNDBUF
    // THERES MORE but only this is needed rn for my needs
}OSNetSocketOptionName;

typedef struct OSNetAddr
{
    OSNetAddrKind kind;
    
    u16 port;
    union
    {
        u8 bytes[16];
        struct
        {
            u32 ipv4;
            u32 _unused1;
            u32 _unused2;
            u32 _unused3;
        };
        struct
        {
            u32 words[4];
        };
    }addr;

    struct OSNetAddr *next;
    struct OSNetAddr *prev;
}OSNetAddr;

typedef struct OSNetAddrInfo
{
    OSNetAddr addr;
    OSNetSocketKind sockKind;
    OSNetProtocolKind protoKind;
    
    struct OSNetAddrInfo *next;
    struct OSNetAddrInfo *prev;
}OSNetAddrInfo;

typedef enum OSNetPollRequestFlag
{
    OS_NET_POLL_REQ_RECIEVE_FLAG = 1 << 0,
    OS_NET_POLL_REQ_SEND_FLAG = 1 << 1, //DONT USE  this with connectionless sockets, like udp or raw, or icmp , they are always ready to send
}OSNetPollRequestFlag;

typedef enum OSNetPollReadyFlag
{
    OS_NET_POLL_READY_RECIEVE_FLAG = OS_NET_POLL_REQ_RECIEVE_FLAG,
    OS_NET_POLL_READY_SEND_FLAG = OS_NET_POLL_REQ_SEND_FLAG,
}OSNetPollReadyFlag;

typedef struct OSNetPollData
{
    OSHandle socketHandle;
    OSNetPollRequestFlag requestFlags;
    OSNetPollReadyFlag readyFlags;

    struct OSNetPollData *next;
    struct OSNetPollData *prev;
}OSNetPollData;

#pragma pack(push, 1)
typedef struct OSNetIPHeader
{
    u8 ihl : 4;         // Internet Header Length (number of 32-bit words)
    u8 version : 4;     // Version (IPv4 = 4)
    u8 tos;             // Type of Service
    u16 totalLen;        // Total Length (header + payload, in bytes)
    u16 id;             // Identification
    u16 fragOff;       // Flags and Fragment Offset
    u8 ttl;             // Time to Live
    u8 protocol;        // Protocol (e.g., TCP, UDP, ICMP)
    u16 check;          // Header checksum
    u32 sender;          // Source IP address
    u32 dest;          // Destination IP address
}OSNetIPHeader;
typedef struct OSNetICMPCommonHeader
{
    u8 type;
    u8 code;
    u16 checksum;
}OSNetICMPCommonHeader;

typedef struct OSNetICMPEcho
{
    OSNetICMPCommonHeader common;
    u16 id;
    u16 seq;
}OSNetICMPEcho;
#pragma pack(pop)

BASE_CREATE_EFFICIENT_LL_DECLS(OSNetAddrInfoList, OSNetAddrInfo);
BASE_CREATE_EFFICIENT_LL_DECLS(OSNetAddrList, OSNetAddr);

BASE_CREATE_ARRAY_VIEW_DECLS_DEFS(OSNetPollDataArray, OSNetPollData);
BASE_CREATE_EFFICIENT_LL_DECLS(OSNetPollDataList, OSNetPollData);

u16 OSNetChecksum(u16 *data, u64 len);

bool OSNetInit(void);

OSNetAddrInfoList OSNetGetAddrInfo(Arena *arena, str8 addr, str8 port, OSNetAddrInfo *hint);
str8 OSNetAddrToStr8(Arena *arena, OSNetAddr addr);
OSNetAddr OSNetStr8ToAddr(str8 addr, OSNetAddrKind kind);

OSNetAddr OSNetGetLocalIpAddressForDest(OSNetAddr dest);
OSNetAddrList OSNetGetLocalIpAddress(Arena *arena, OSNetAddrKind preference);
OSNetAddr OSNetGetPublicIpAddress(void);

OSHandle OSNetSocketCreate(OSNetAddrKind family, OSNetSocketKind socketKind, OSNetProtocolKind protocolKind);
OSHandle OSNetSocketCreateFromAddrInfo(OSNetAddrInfo *info);
OSHandleList OSNetSocketCreateFromAddr(Arena *arena, str8 addr, str8 port, OSNetAddrInfo *hint);

bool OSNetSocketGetOptions(OSHandle socketHandle, OSNetSocketOptionLevel level, OSNetSocketOptionName name, void *value, u64 *valueLen);
bool OSNetSocketSetOptions(OSHandle socketHandle, OSNetSocketOptionLevel level, OSNetSocketOptionName name, void *value, u64 valueLen);
bool OSNetSocketBind(OSHandle socketHandle, OSNetAddr bindingAddr);
bool OSNetSocketListen(OSHandle socketHandle);
OSHandle OSNetSocketAccept(OSHandle socketHandle, OSNetAddr *acceptedAddr);
bool OSNetSocketConnect(OSHandle socketHandle, OSNetAddr connectTo);

i64 OSNetSocketSendTo(OSHandle socketHandle, U8Array buf, OSNetAddr to);
i64 OSNetSocketRecieveFrom(OSHandle socketHandle, U8Array *outBuf, OSNetAddr *recievedFrom);

i64 OSNetSocketSendAll(OSHandle socketHandle, U8Array buf);
i64 OSNetSocketSend(OSHandle socketHandle, U8Array buf);
i64 OSNetSocketRecieve(OSHandle socketHandle, U8Array *outBuf);
U8ChunkList OSNetSocketRecieveAll(Arena *arena, OSHandle socketHandle);

bool OSNetSocketClose(OSHandle socketHandle);

i64 OSNetSocketPollArray(OSNetPollDataArray* socketsToPoll, i64 timeoutMS);
i64 OSNetSocketPollList(OSNetPollDataList socketsToPoll, i64 timeoutMS);

#endif