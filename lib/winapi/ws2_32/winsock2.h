#ifndef __WINSOCK2_H__
#define __WINSOCK2_H__

#include <windef.h>
#include <ws2def.h>
#include <basetsd.h>

#define WSAAPI FAR PASCAL
#define WSAOVERLAPPED OVERLAPPED
typedef struct _OVERLAPPED *LPWSAOVERLAPPED;

typedef unsigned char u_char;
typedef unsigned short u_short;
typedef unsigned int u_int;
typedef unsigned long u_long;

typedef UINT_PTR SOCKET;
#define INVALID_SOCKET (SOCKET)(~0)
#define SOCKET_ERROR (-1)

#ifndef FD_SETSIZE
#define FD_SETSIZE 64
#endif

typedef struct fd_set
{
    u_int fd_count;
    SOCKET fd_array[FD_SETSIZE];
} fd_set;

typedef struct WSAData
{
    WORD wVersion;
    WORD wHighVersion;
} WSADATA, FAR *LPWSADATA; // FIXME

int WSAAPI WSAStartup (WORD wVersionRequested, LPWSADATA lpWSAData);
int WSAAPI WSACleanup (void);

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

int WSAAPI WSAHtonl (SOCKET s, u_long hostlong, u_long FAR *lpnetlong);
int WSAAPI WSAHtons (SOCKET s, u_short hostshort, u_short FAR *lpnetshort);

int WSAAPI WSAGetLastError (void);
void WSAAPI WSASetLastError (int iError);

int WSAAPI closesocket (SOCKET s);

int WSAAPI recv (SOCKET s, char *buf, int len, int flags);
int WSAAPI recvfrom (SOCKET s, char *buf, int len, int flags, struct sockaddr *from, int *fromlen);
int WSAAPI send (SOCKET s, const char *buf, int len, int flags);
int WSAAPI sendto (SOCKET s, const char *buf, int len, int flags, const struct sockaddr *to, int tolen);

typedef void (CALLBACK *LPWSAOVERLAPPED_COMPLETION_ROUTINE) (DWORD dwError, DWORD cbTransferred, LPWSAOVERLAPPED lpOverlapped, DWORD dwFlags);

int WSAAPI WSARecvFrom (SOCKET s, LPWSABUF lpBuffers, DWORD dwBufferCount, LPDWORD lpNumberOfBytesRecvd, LPDWORD lpFlags, struct sockaddr FAR *lpFrom, LPINT lpFromlen, LPWSAOVERLAPPED lpOverlapped, LPWSAOVERLAPPED_COMPLETION_ROUTINE lpCompletionRoutine);
int WSAAPI WSASendTo (SOCKET s, LPWSABUF lpBuffers, DWORD dwBufferCount, LPDWORD lpNumberOfBytesSent, DWORD dwFlags, const struct sockaddr FAR *lpTo, int ITolen, LPWSAOVERLAPPED lpOverlapped, LPWSAOVERLAPPED_COMPLETION_ROUTINE lpCompletionRoutine);


#endif
