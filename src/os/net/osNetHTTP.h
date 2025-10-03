#ifndef OS_NET_HTTP_H
#define OS_NET_HTTP_H
#include "base\baseCore.h"
#include "base\baseStrings.h"
#include "base\baseMemory.h"
#include "base\baseThreads.h"

typedef struct OSNetHttpHeader
{
    str8 name;
    str8 value;

    struct OSNetHttpHeader *next;
    struct OSNetHttpHeader *prev;
}OSNetHttpHeader;

BASE_CREATE_EFFICIENT_LL_DECLS(OSNetHttpHeaderList, OSNetHttpHeader);

typedef struct OSNetHttpPacket
{
    U8Array messageBody;
    str8 method;
    str8 resource;
    str8 httpVersion;
    str8 statusReason;
    
    u16 status;

    OSNetHttpHeaderList headers;
}OSNetHttpPacket;

OSNetHttpHeader OSNetHttpHeaderFromStr8(Arena *arena, str8 str);
OSNetHttpHeaderList OSNetHttpHeaderListFromStr8(Arena *arena, str8 str);
OSNetHttpPacket OSNetHttpPacketFromStr8(Arena *arena, str8 str);
#endif