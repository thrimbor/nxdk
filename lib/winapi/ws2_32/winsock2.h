#ifndef __WINSOCK2_H__
#define __WINSOCK2_H__

int WSAAPI WSAHtonl (SOCKET s, u_long hostlong, U_long *lpnetlong);
int WSAAPI WSAHtons (SOCKET s, u_short hostshort, U_short *lpnetshort);

int WSAAPI WSAGetLastError();
void WSAAPI WSASetLastError (int iError);

int WSAAPI WSARecvFrom (SOCKET s, LPWSABUF lpBuffers, DWORD dwBufferCount, LPDWORD lpNumberOfBytesRecvd, LPDWORD lpFlags, sockaddr *lpFrom, LPINT lpFromlen, LPWSAOVERLAPPED lpOverlapped, LPWSAOVERLAPPED_COMPLETION_ROUTINE lpCompletionRoutine);
int WSAAPI WSASendTo (SOCKET s, LPWSABUF lpBuffers, DWORD dwBufferCount, LPDWORD lpNumberOfBytesSent, DWORD dwFlags, const sockaddr *lpTo, int ITolen, LPWSAOVERLAPPED lpOverlapped, LPWSAOVERLAPPED_COMPLETION_ROUTINE lpCompletionRoutine);


#endif
