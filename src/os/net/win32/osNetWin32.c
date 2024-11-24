#include "os\net\osNet.h"
#include "osNetWin32.h"

OSNetAddrKind OSNetWin32AddrFamilyToAddrKind(int addrKind)
{
    switch(addrKind)
    {
        case AF_INET: return OS_NET_ADDR_IPV4;
        case AF_INET6: return OS_NET_ADDR_IPV6;
        case AF_UNSPEC: return OS_NET_ADDR_EITHER;
        default:
        {
            logThreadErrorFmt("Unregognised addr kind specified.\n");
            return OS_NET_ADDR_EITHER;
        }break;
    }
}
int OSNetAddrKindToWin32AddrFamily(OSNetAddrKind addrKind)
{
    switch(addrKind)
    {
        case OS_NET_ADDR_IPV4: return AF_INET;
        case OS_NET_ADDR_IPV6: return AF_INET6;
        case OS_NET_ADDR_EITHER: return AF_UNSPEC;
        default:
        {
            logThreadErrorFmt("Unregognised addr kind specified.\n");
            return AF_UNSPEC;
        }break;
    }
}
OSNetProtocolKind OSNetWin32IPPROTOToProtocolKind(IPPROTO protocol)
{
    switch(protocol)
    {
        case IPPROTO_TCP: return OS_NET_PROTOCOL_TCP;
        case IPPROTO_UDP: return OS_NET_PROTOCOL_UDP;
        case IPPROTO_ICMP: return OS_NET_PROTOCOL_ICMP;
        case IPPROTO_RAW: return OS_NET_PROTOCOL_RAW;
        default:
        {
            logThreadErrorFmt("Unregognised protocol specified.\n");
            return 0;
        }break;
    }
}
IPPROTO OSNetProtocolKindToWin32IPPROTO(OSNetProtocolKind protocol)
{
    switch(protocol)
    {
        case OS_NET_PROTOCOL_TCP: return IPPROTO_TCP;
        case OS_NET_PROTOCOL_UDP: return IPPROTO_UDP;
        case OS_NET_PROTOCOL_ICMP: return IPPROTO_ICMP;
        case OS_NET_PROTOCOL_RAW: return IPPROTO_RAW;
        default:
        {
            logThreadErrorFmt("Unregognised protocol specified.\n");
            return IPPROTO_IP;
        }break;
    }
}

OSNetSocketKind OSNetWin32SockTypeToSocketKind(int sock)
{
    switch(sock)
    {
        case SOCK_STREAM: return OS_NET_SOCK_STREAM;
        case SOCK_DGRAM: return OS_NET_SOCK_DATAGRAM;
        case SOCK_RAW: return OS_NET_SOCK_RAW;
        default:
        {
            logThreadErrorFmt("Unregognised socket type specified.\n");
            return 0;
        }break;
    }
}
int OSNetSocketKindToWin32SockType(OSNetSocketKind sock)
{
    switch(sock)
    {
        case OS_NET_SOCK_STREAM: return SOCK_STREAM;
        case OS_NET_SOCK_DATAGRAM: return SOCK_DGRAM;
        case OS_NET_SOCK_RAW: return SOCK_RAW;
        default:
        {
            logThreadErrorFmt("Unregognised socket kind specified.\n");
            return 0;
        }break;
    }
}

int OSNetAddrToWin32SOCKADDR(OSNetAddr addr, SOCKADDR_STORAGE* win32Addr)
{
    int size = 0;
    if (addr.kind == OS_NET_ADDR_IPV4)
    {
        ((SOCKADDR_IN*)win32Addr)->sin_family = AF_INET;
        ((SOCKADDR_IN*)win32Addr)->sin_port = addr.port;
        ((SOCKADDR_IN*)win32Addr)->sin_addr.S_un.S_addr = addr.addr.ipv4;

        size = sizeof(SOCKADDR_IN);
    }
    else
    {
        ((SOCKADDR_IN6*)win32Addr)->sin6_family = AF_INET6;
        ((SOCKADDR_IN6*)win32Addr)->sin6_port = addr.port;
        BASE_MEMCPY(((SOCKADDR_IN6*)win32Addr)->sin6_addr.u.Byte, addr.addr.bytes, 16);
        size = sizeof(SOCKADDR_IN6);
    }

    return size;
}
void OSNetWin32SOCKADDRToAddr(SOCKADDR_STORAGE *win32Addr, OSNetAddr *addr)
{
    if (win32Addr->ss_family == AF_INET)
    {
        addr->addr.ipv4 = ((SOCKADDR_IN*) win32Addr)->sin_addr.S_un.S_addr;
        addr->port = ((SOCKADDR_IN*) win32Addr)->sin_port;
        addr->kind = OS_NET_ADDR_IPV4;
    }
    else
    {
        BASE_MEMCPY(addr->addr.bytes, ((SOCKADDR_IN6*) win32Addr)->sin6_addr.u.Byte, 16);
        addr->port = ((SOCKADDR_IN6*) win32Addr)->sin6_port;
        addr->kind = OS_NET_ADDR_IPV6;
    }
}

enum OSNetSocketOptionLevel OSNetWin32OptionLevelToSocketOptionLevel(int level)
{
    switch (level)
    {
        case IPPROTO_IP: return OS_NET_SOCK_OPTION_IPV4;
        default:
        {
            logThreadErrorFmt("Unregognised socket optional level.\n");
            return 0;
        }break;
    }
}
int OSNetSocketOptionLevelToWin32OptionLevel(enum OSNetSocketOptionLevel level)
{
    switch (level)
    {
        case OS_NET_SOCK_OPTION_IPV4: return IPPROTO_IP;
        default:
        {
            logThreadErrorFmt("Unregognised socket optional level.\n");
            return 0;
        }break;
    }
}
enum OSNetSocketOptionName OSNetWin32OptionNameToSocketOptionName(int name)
{
    switch (name)
    {
        case IP_HDRINCL: return OS_NET_SOCK_OPTION_IPV4__IP_HEADER_INCLUDE;
        default:
        {
            logThreadErrorFmt("Unregognised socket optional name.\n");
            return 0;
        }break;
    }
}
int OSNetSocketOptionNameToWin32OptionName(enum OSNetSocketOptionName name)
{
    switch (name)
    {
        case OS_NET_SOCK_OPTION_IPV4__IP_HEADER_INCLUDE: return IP_HDRINCL;
        default:
        {
            logThreadErrorFmt("Unregognised socket optional name.\n");
            return 0;
        }break;
    }
}

bool OSNetInit(void)
{
    WSADATA wsaData = {0};
    if (WSAStartup(MAKEWORD(2, 2), &wsaData))
    {
        logThreadErrorFmt("Failed to initialize winsock with version 2.2");
        return false;
    }

    if (wsaData.wVersion != MAKEWORD(2, 2))
    {
        logThreadErrorFmt("Failed to initialize winsock with version 2.2");
        WSACleanup();
        return false;
    }

    return true;
}

OSNetAddrInfoList OSNetGetAddrInfo(BaseArena *arena, str8 addr, str8 port, OSNetAddrInfo *hint)
{
    ADDRINFOA win32AddrInfoHint = 
    {
        .ai_family = AF_UNSPEC,
        .ai_socktype = SOCK_STREAM,
        .ai_protocol = IPPROTO_TCP,
    };
    
    i8 *addrData = (i8*)addr.data;
    i8 *portData = (i8*)port.data;

    if (BASE_NULL_OR_EMPTY(addr))
    {
        addrData = null;
        win32AddrInfoHint.ai_flags = AI_PASSIVE;
    }

    if (BASE_NULL_OR_EMPTY(port))
    {
        portData = null;
    }

    if (hint)
    {
        win32AddrInfoHint.ai_family = OSNetAddrKindToWin32AddrFamily(hint->addr.kind);
        win32AddrInfoHint.ai_protocol= OSNetProtocolKindToWin32IPPROTO(hint->protoKind);
        win32AddrInfoHint.ai_socktype= OSNetSocketKindToWin32SockType(hint->sockKind);
    }

    OSNetAddrInfoList ret = {0};

    ADDRINFOA *win32Ret = null;
    if(getaddrinfo(addrData, portData, &win32AddrInfoHint, &win32Ret) == 0)
    {
        ADDRINFOA *win32CurrentAddr = win32Ret;
        while(win32CurrentAddr != null)
        {
            OSNetAddrInfo *info = baseArenaPushType(arena, OSNetAddrInfo);
            OSNetWin32SOCKADDRToAddr((SOCKADDR_STORAGE*) win32CurrentAddr->ai_addr, &info->addr);

            info->protoKind = OSNetWin32IPPROTOToProtocolKind(win32CurrentAddr->ai_protocol);
            info->sockKind = OSNetWin32SockTypeToSocketKind(win32CurrentAddr->ai_socktype);

            BaseListNodePushLast(ret, info);

            win32CurrentAddr = win32CurrentAddr->ai_next;
        }

        freeaddrinfo(win32Ret);
    }

    return ret;
}

str8 OSNetAddrToStr8(BaseArena *arena, OSNetAddr addr)
{
    str8 ret = {0};
    if (addr.kind == OS_NET_ADDR_IPV4)
    {
        ret.data = baseArenaPushArray(arena, u8, INET_ADDRSTRLEN);
        ret.len = INET_ADDRSTRLEN;

        inet_ntop(AF_INET, &addr.addr.ipv4, (i8*)ret.data, ret.len - 1);

        ret.len = strlen((i8*)ret.data);
    }
    else
    {
        ret.data = baseArenaPushArray(arena, u8, INET6_ADDRSTRLEN);
        ret.len = INET6_ADDRSTRLEN;

        inet_ntop(AF_INET6, &addr.addr.bytes, (i8*)ret.data, ret.len - 1);

        ret.len = strlen((i8*)ret.data);
    }

    return ret;
}
OSNetAddr OSNetStr8ToAddr(str8 addr, OSNetAddrKind kind)
{
    OSNetAddr ret = {0};

    if (kind == OS_NET_ADDR_IPV4)
    {
        inet_pton(AF_INET, (i8*)addr.data, &ret.addr.ipv4);
    }
    else
    {
        inet_pton(AF_INET6, (i8*)addr.data, ret.addr.bytes);
    }

    return ret;
}

OSNetAddrList OSNetGetLocalIpAddress(BaseArena *arena, OSNetAddrKind preference)
{
    OSNetAddrList addrList = {0};

    MIB_IPFORWARD_TABLE2 *table = null;
    DWORD result = GetIpForwardTable2(AF_UNSPEC, &table);
    if (result == NO_ERROR)
    {
        u64 interfaceIndex = 0;
        for(u64 i = 0; i < table->NumEntries; i++)
        {
            MIB_IPFORWARD_ROW2 entry = table->Table[i];

            if (entry.DestinationPrefix.PrefixLength == 0)
            {
                interfaceIndex = entry.InterfaceIndex;
                break;
            }
        }
        FreeMibTable(table);

        BaseArenaTemp temp = baseTempBegin(&arena, 1);
        {
            ULONG adaptersLength = 1;
            IP_ADAPTER_ADDRESSES *adapters = baseArenaPushArray(temp.arena, IP_ADAPTER_ADDRESSES, adaptersLength);

            result = GetAdaptersAddresses(AF_UNSPEC, 
                                            GAA_FLAG_SKIP_MULTICAST | GAA_FLAG_SKIP_ANYCAST,
                                            null,
                                            adapters,
                                            &adaptersLength);

            if (result == ERROR_BUFFER_OVERFLOW)
            {
                adapters = baseArenaPushArray(arena, IP_ADAPTER_ADDRESSES, adaptersLength);
                result = GetAdaptersAddresses(AF_UNSPEC, 
                                            GAA_FLAG_SKIP_MULTICAST | GAA_FLAG_SKIP_ANYCAST,
                                            null,
                                            adapters,
                                            &adaptersLength);
            }

            if (result == ERROR_SUCCESS)
            {
                while (adapters != null)
                {
                    if (adapters->OperStatus == IfOperStatusUp && adapters->IfIndex == interfaceIndex)
                    {
                        IP_ADAPTER_UNICAST_ADDRESS *currentUnicast = adapters->FirstUnicastAddress;
                        while (currentUnicast != null)
                        {
                            if ((currentUnicast->Address.lpSockaddr->sa_family == AF_INET) &&
                                (preference == OS_NET_ADDR_IPV4 || preference == OS_NET_ADDR_EITHER))
                            {
                                OSNetAddr *addr = baseArenaPushType(arena, OSNetAddr);

                                addr->kind = OS_NET_ADDR_IPV4;
                                addr->addr.ipv4 = (u32)((SOCKADDR_IN*) currentUnicast->Address.lpSockaddr)->sin_addr.S_un.S_addr;
                                addr->port = ((SOCKADDR_IN*) currentUnicast->Address.lpSockaddr)->sin_port;

                                BaseListNodePushLast(addrList, addr);
                            }
                            else if (preference == OS_NET_ADDR_IPV6 || preference == OS_NET_ADDR_EITHER)
                            {
                                OSNetAddr *addr = baseArenaPushType(arena, OSNetAddr);

                                addr->kind = OS_NET_ADDR_IPV6;
                                BASE_MEMCPY(addr->addr.bytes, ((SOCKADDR_IN6*) currentUnicast->Address.lpSockaddr)->sin6_addr.u.Byte, 16);
                                addr->port = ((SOCKADDR_IN6*) currentUnicast->Address.lpSockaddr)->sin6_port;

                                BaseListNodePushLast(addrList, addr);
                            }
                            
                            currentUnicast = currentUnicast->Next;
                        }
                    }

                    adapters = adapters->Next;
                }
            }
        }

        baseTempEnd(temp);
    }

    return addrList;
}

OSHandle OSNetSocketCreate(OSNetAddrKind family, OSNetSocketKind socketKind, OSNetProtocolKind protocolKind)
{
    OSHandle handle = {._u64 = 0};

    int af = OSNetAddrKindToWin32AddrFamily(family);
    int s = OSNetSocketKindToWin32SockType(socketKind);
    int p = OSNetProtocolKindToWin32IPPROTO(protocolKind);

    SOCKET sock = socket(af, s, p);

    if (sock != INVALID_SOCKET)
    {
        handle._u64 = (u64)sock;
    }

    return handle;
}
OSHandle OSNetSocketCreateFromAddrInfo(OSNetAddrInfo *info)
{
    return OSNetSocketCreate(info->addr.kind, info->sockKind, info->protoKind);
}
OSHandleList OSNetSocketCreateFromAddr(BaseArena *arena, str8 addr, str8 port, OSNetAddrInfo *hint)
{
    OSHandleList handles = {0};

    BaseArenaTemp temp = baseTempBegin(&arena, 1);
    {
        OSNetAddrInfoList addrInfos = OSNetGetAddrInfo(temp.arena, addr, port, hint);

        BASE_LIST_FOREACH(OSNetAddrInfo, addrInfo, addrInfos)
        {
            OSHandle *handle = baseArenaPushType(arena, OSHandle);
            *handle = OSNetSocketCreateFromAddrInfo(addrInfo);
            BaseListNodePushLast(handles, handle);
        }
    }
    baseTempEnd(temp);

    return handles;
}
bool OSNetSocketSetOptions(OSHandle socketHandle, OSNetSocketOptionLevel level, OSNetSocketOptionName name, void *value, u64 valueLen)
{
    int win32Level = OSNetSocketOptionLevelToWin32OptionLevel(level);
    int win32Name = OSNetSocketOptionNameToWin32OptionName(name);

    return setsockopt((SOCKET)socketHandle._u64, win32Level, win32Name, value, (int)valueLen) != SOCKET_ERROR;
}
bool OSNetSocketBind(OSHandle socketHandle, OSNetAddr bindingAddr)
{
    SOCKADDR_STORAGE addr = {0};
    int size = OSNetAddrToWin32SOCKADDR(bindingAddr, &addr);
    return bind((SOCKET)socketHandle._u64, (SOCKADDR*)&addr, size) != SOCKET_ERROR;
}
bool OSNetSocketListen(OSHandle socketHandle)
{
    return listen((SOCKET)socketHandle._u64, SOMAXCONN) != SOCKET_ERROR;
}
OSHandle OSNetSocketAccept(OSHandle socketHandle, OSNetAddr *acceptedAddr)
{
    SOCKADDR_STORAGE win32Accepted = {0};
    int size = sizeof(win32Accepted);

    SOCKET win32NewSocket = accept((SOCKET)socketHandle._u64, (SOCKADDR*)&win32Accepted, &size);
    OSNetWin32SOCKADDRToAddr(&win32Accepted, acceptedAddr);
    
    return (OSHandle){._u64 = (u64)win32NewSocket};
}
bool OSNetSocketConnect(OSHandle socketHandle, OSNetAddr connectTo)
{
    SOCKADDR_STORAGE win32ConnectTo = {0};
    int size = OSNetAddrToWin32SOCKADDR(connectTo, &win32ConnectTo);

    return connect((SOCKET)socketHandle._u64, (SOCKADDR*)&win32ConnectTo, size) != SOCKET_ERROR;
}
i64 OSNetSocketSendTo(OSHandle socketHandle, U8Array buf, OSNetAddr to)
{
    SOCKADDR_STORAGE addr = {0};
    int size = OSNetAddrToWin32SOCKADDR(to, &addr);

    int sent = sendto((SOCKET)socketHandle._u64, (i8*)buf.data, (int)buf.len, 0, (SOCKADDR*)&addr, size);
    // int lastError = WSAGetLastError();

    return sent;
}
i64 OSNetSocketRecieveFrom(OSHandle socketHandle, U8Array *outBuf, OSNetAddr *recievedFrom)
{
    SOCKADDR_STORAGE win32RecievedFrom = {0};
    int fromLen = sizeof(win32RecievedFrom);
    int recieved = recvfrom((SOCKET)socketHandle._u64, (i8*)outBuf->data, (int)outBuf->len, 0, (SOCKADDR*)&win32RecievedFrom, &fromLen);

    // int lastError = WSAGetLastError();

    OSNetWin32SOCKADDRToAddr(&win32RecievedFrom, recievedFrom);

    return recieved;
}
i64 OSNetSocketSend(OSHandle socketHandle, U8Array buf)
{
    int sent = send((SOCKET)socketHandle._u64, (i8*)buf.data, (int)buf.len, 0);
    return sent;
}
i64 OSNetSocketRecieve(OSHandle socketHandle, U8Array *outBuf)
{
    int recieved = recv((SOCKET)socketHandle._u64, (i8*)outBuf->data, (int)outBuf->len, 0);
    return recieved;
}
bool OSNetSocketClose(OSHandle socketHandle)
{
    bool success = closesocket((SOCKET)socketHandle._u64) == 0;

    return success;
}
// i64 OSNetSocketPollArray(OSNetPollDataArray* socketsToPoll, i64 timeoutMS)
// {
//     i64 count = 0;
//     BaseArenaTemp temp = baseTempBegin(null, 0);
//     {
//         WSAPOLLFD *pollData = baseArenaPushArray(temp.arena, WSAPOLLFD, socketsToPoll->len);
//         for (u64 i = 0; i < socketsToPoll->len; i++)
//         {
//             OSNetPollData socketToPoll = socketsToPoll->data[i];
//             pollData[i].fd = (SOCKET)socketToPoll.socketHandle._u64;

//             if (socketToPoll.requestFlags & OS_NET_POLL_REQ_RECIEVE_FLAG) pollData[i].events |= POLLIN;
//             if (socketToPoll.requestFlags & OS_NET_POLL_REQ_SEND_FLAG) pollData[i].events |= POLLOUT;

//             if (socketToPoll.readyFlags & OS_NET_POLL_READY_RECIEVE_FLAG) pollData[i].revents |= POLLIN;
//             if (socketToPoll.readyFlags & OS_NET_POLL_READY_SEND_FLAG) pollData[i].revents |= POLLOUT;
//         }

//         count = WSAPoll(pollData, (ULONG)socketsToPoll->len, (INT)timeoutMS);
//         if (count > 0)
//         {
//             for (u64 i = 0; i < socketsToPoll->len; i++)
//             {
//                 if (pollData[i].revents & POLLIN)
//                 {
//                     socketsToPoll->data[i].readyFlags |= OS_NET_POLL_READY_RECIEVE_FLAG;
//                 }

//                 if (pollData[i].revents & POLLOUT)
//                 {
//                     socketsToPoll->data[i].readyFlags |= OS_NET_POLL_READY_SEND_FLAG;
//                 }
//             }
//         }
//     }
//     baseTempEnd(temp);

//     return count;
// }
i64 OSNetSocketPollList(OSNetPollDataList socketsToPoll, i64 timeoutMS)
{
    i64 count = 0;
    BaseArenaTemp temp = baseTempBegin(null, 0);
    {
        WSAPOLLFD *pollData = baseArenaPushArray(temp.arena, WSAPOLLFD, socketsToPoll.len);

        u64 index = 0;
        BASE_LIST_FOREACH_INDEX(OSNetPollData, socketToPoll, socketsToPoll, index)
        {
            pollData[index].fd = (SOCKET)socketToPoll->socketHandle._u64;

            if (socketToPoll->requestFlags & OS_NET_POLL_REQ_RECIEVE_FLAG) pollData[index].events |= POLLIN;
            if (socketToPoll->requestFlags & OS_NET_POLL_REQ_SEND_FLAG) pollData[index].events |= POLLOUT;

            if (socketToPoll->readyFlags & OS_NET_POLL_READY_RECIEVE_FLAG) pollData[index].revents |= POLLIN;
            if (socketToPoll->readyFlags & OS_NET_POLL_READY_SEND_FLAG) pollData[index].revents |= POLLOUT;

            pollData[index].revents = 0;
        }

        count = WSAPoll(pollData, (ULONG)socketsToPoll.len, (INT)timeoutMS);
        if (count > 0)
        {
            index = 0;
            BASE_LIST_FOREACH_INDEX(OSNetPollData, socketToPoll, socketsToPoll, index)
            {
                socketToPoll->readyFlags = 0;

                if (pollData[index].revents & POLLIN)
                {
                    socketToPoll->readyFlags |= OS_NET_POLL_READY_RECIEVE_FLAG;
                }

                if (pollData[index].revents & POLLOUT)
                {
                    socketToPoll->readyFlags |= OS_NET_POLL_READY_SEND_FLAG;
                }
            }
        }
    }
    baseTempEnd(temp);

    return count;
}
