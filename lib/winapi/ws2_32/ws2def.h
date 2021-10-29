#ifndef __WS2DEF_H__
#define __WS2DEF_H__

#include <windef.h>

typedef USHORT ADDRESS_FAMILY;

typedef struct sockaddr
{
    ADDRESS_FAMILY sa_family;
    CHAR sa_data[14];
} SOCKADDR, *PSOCKADDR, FAR *LPSOCKADDR;

typedef struct _WSABUF
{
    ULONG len;
    CHAR FAR *buf;
} WSABUF, *LPWSABUF;

#endif
