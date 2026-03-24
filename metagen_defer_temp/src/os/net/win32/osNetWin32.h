#pragma comment(lib, "iphlpapi.lib")
#pragma comment(lib, "Ws2_32.lib")

#include <WinSock2.h>
#include <WS2tcpip.h>
#include <iphlpapi.h>

enum OSNetAddrKind OSNetWin32AddrFamilyToAddrKind(int addrKind);
int OSNetAddrKindToWin32AddrFamily(enum OSNetAddrKind addrKind);

enum OSNetProtocolKind OSNetWin32IPPROTOToProtocolKind(IPPROTO protocol);
IPPROTO OSNetProtocolKindToWin32IPPROTO(enum OSNetProtocolKind protocol);

enum OSNetSocketKind OSNetWin32SockTypeToSocketKind(int sock);
int OSNetSocketKindToWin32SockType(enum OSNetSocketKind sock);

void OSNetWin32SOCKADDRToAddr(SOCKADDR_STORAGE *win32Addr, struct OSNetAddr *addr);
int OSNetAddrToWin32SOCKADDR(struct OSNetAddr addr, SOCKADDR_STORAGE* win32Addr);

enum OSNetSocketOptionLevel OSNetWin32OptionLevelToSocketOptionLevel(int level);
int OSNetSocketOptionLevelToWin32OptionLevel(enum OSNetSocketOptionLevel level);

enum OSNetSocketOptionName OSNetWin32OptionNameToSocketOptionName(int name);
int OSNetSocketOptionNameToWin32OptionName(enum OSNetSocketOptionName name);