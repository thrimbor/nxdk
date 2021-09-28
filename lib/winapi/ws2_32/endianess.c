#include <winsock2.h>

unsigned __int64 htond (double Value)
{
    return __builtin_bswap64(*(double *)&Value);
}

unsigned __int32 htonf (float Value)
{
    return __builtin_bswap32(*(float *)&Value);
}

u_long WSAAPI htonl (u_long hostlong)
{
    return __builtin_bswap32(hostlong);
}

unsigned __int64 htonll (unsigned __int64 Value)
{
    return __builtin_bswap64(Value);
}

u_short WSAAPI htons (u_short hostshort)
{
    return __builtin_bswap16(hostshort);
}

double ntohd (unsigned __int64 Value)
{
    __int64 v = __builtin_bswap64(Value);
    return *(double *)&v;
}

float ntohf (unsigned __int32 Value)
{
    __int32 v = __builtin_bswap32(Value);
    return *(float *)&v;
}

u_long WSAAPI ntohl (u_long netlong)
{
    return __builtin_bswap32(netlong);
}

unsigned __int64 ntohll (unsigned __int64 Value)
{
    return __builtin_bswap64(Value);
}

u_short WSAAPI ntohs (u_short netshort)
{
    return __builtin_bswap16(netshort);
}

//int WSAAPI WSAHtonl (SOCKET s, u_long hostlong, u_long *lpnetlong);
//int WSAAPI WSAHtons (SOCKET s, u_short hostshort, u_short *lpnetshort);
