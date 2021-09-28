#ifndef __WINSOCK2_H__
#define __WINSOCK2_H__

#include <windef.h>

#define WSAAPI FAR PASCAL

typedef unsigned char u_char;
typedef unsigned short u_short;
typedef unsigned int u_int;
typedef unsigned long u_long;

unsigned __int64 htond (double Value);
unsigned __int32 htonf (float Value);
u_long WSAAPI htonl (u_long hostlong);
unsigned __int64 htonll (unsigned __int64 Value);
u_short WSAAPI htons (u_short hostshort);
double ntohd (unsigned __int64 Value);
float ntohf (unsigned __int32 Value);
u_long WSAAPI ntohl (u_long netlong);
unsigned __int64 ntohll (unsigned __int64 Value);
u_short WSAAPI ntohs (u_short netshort);

#endif
