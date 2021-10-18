#include <assert.h>
#include <hal/debug.h>
#include <hal/video.h>
#include <nxdk/net.h>
#include <winsock2.h>
#include <windows.h>

void Connect()
{
    WSADATA WsaData;
    int iResult = WSAStartup( MAKEWORD(2,2), &WsaData );
    if( iResult != NO_ERROR )
    debugPrint("Error at WSAStartup\n");

    char aa[5000];
    SOCKET m_socket;
    m_socket = socket(AF_INET,SOCK_STREAM,IPPROTO_TCP);

    int  whathappened=WSAGetLastError ();
    debugPrint("SOCKET = %ld\n", m_socket);
    debugPrint("Whathappened = %ld\n", whathappened);

    if (m_socket == INVALID_SOCKET) {
        debugPrint("Error at socket()\n");
        WSACleanup();
        return;
    }

    sockaddr_in service;
    service.sin_family = AF_INET;
    service.sin_addr.s_addr = inet_addr("192.168.1.10");
    service.sin_port = htons(80);

    int results=connect(m_socket,(sockaddr*) &service,sizeof(struct sockaddr));
    if( results == SOCKET_ERROR)
    {
        debugPrint("Error at connect()\n");
    }

    send(m_socket, "GET / \r\n", strlen("GET / \r\n"), 0);
    int rr = 1;
    while (rr)
    {
        rr = recv(m_socket, aa, 500, 0);
        debugPrint("recv: %s\n", aa);
    }
}

int main(void)
{
    XVideoSetMode(640, 480, 32, REFRESH_DEFAULT);

    int res = nxNetStartup(NULL);
    assert(res == 0);

    while(1) {
        debugPrint("Hello nxdk!\n");
        Sleep(2000);
    }

    return 0;
}
