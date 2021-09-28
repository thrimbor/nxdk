#include <winsock2.h>

static _Thread_local int lastError = 0;

int WSAAPI WSAGetLastError (void)
{
    return lastError;
}

void WSAAPI WSASetLastError (int iError)
{
    lastError = iError;
}
