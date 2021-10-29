#include <winsock2.h>

#include <assert.h>

#define fd_set lwip_fd_set
#define sockaddr lwip_sockaddr
#include <lwip/sockets.h>
#undef fd_set
#undef sockaddr

// https://docs.microsoft.com/en-us/windows/win32/api/winsock2/nf-winsock2-wsasendto

// https://github.com/JayFoxRox/nxdk/commit/f3dd88898180e6e59d6f42956c346615e335208e

int WSAAPI closesocket (SOCKET s)
{
    int ret = lwip_close(s);
    if (ret == -1) {
        WSASetLastError(map_error_to_winsock(errno));
        return SOCKET_ERROR;
    }

    return 0;
}

int WSAAPI recv (SOCKET s, char *buf, int len, int flags)
{
    ssize_t ret = lwip_recv(s, buf, len, flags);
    if (ret == -1) {
        WSASetLastError(map_error_to_winsock(errno));
        return SOCKET_ERROR;
    }

    return ret;
}

int WSAAPI recvfrom (SOCKET s, char *buf, int len, int flags, struct sockaddr *from, int *fromlen)
{
    // FIXME: Convert sockaddr!
    ssize_t ret = lwip_recvfrom(s, buf, len, flags, from, fromlen);
    if (ret == -1) {
        WSASetLastError(map_error_to_winsock(errno));
        return SOCKET_ERROR;
    }

    return ret;
}

int WSAAPI send (SOCKET s, const char *buf, int len, int flags)
{
    ssize_t ret = lwip_send(s, buf, len, flags);
    if (ret == -1) {
        WSASetLastError(map_error_to_winsock(errno));
        return SOCKET_ERROR;
    }

    return ret;
}

int WSAAPI sendto (SOCKET s, const char *buf, int len, int flags, const struct sockaddr *to, int tolen)
{
    // FIXME: Convert sockaddr!
    ssize_t ret = lwip_sendto(s, buf, len, flags, to, tolen);
    if (ret == -1) {
        WSASetLastError(map_error_to_winsock(errno));
        return SOCKET_ERROR;
    }

    return ret;
}

int WSAAPI WSASendTo (SOCKET s, LPWSABUF lpBuffers, DWORD dwBufferCount, LPDWORD lpNumberOfBytesSent, DWORD dwFlags, const struct sockaddr FAR *lpTo, int iTolen, LPWSAOVERLAPPED lpOverlapped, LPWSAOVERLAPPED_COMPLETION_ROUTINE lpCompletionRoutine)
{
    assert(lpBuffers);
    assert(dwBufferCount);
    assert(dwFlags == 0);
    assert(lpTo);
    assert(iTolen);
    assert(lpOverlapped == NULL);
    assert(lpCompletionRoutine == NULL);

    struct iovec *iov = __builtin_alloca(sizeof(struct iovec) * dwBufferCount);
    for (int i = 0; i < dwBufferCount; i++) {
        iov[i].iov_base = lpBuffers[i].buf;
        iov[i].iov_len = lpBuffers[i].len;
    }

    struct msghdr msg;
    msg.msg_name = (void *)lpTo;
    msg.msg_namelen = iTolen;
    msg.msg_iov = iov;
    msg.msg_iovlen = dwBufferCount;
    msg.msg_control = NULL;
    msg.msg_controllen = 0;

    ssize_t ret = lwip_sendmsg(s, &msg, 0);
    if (ret == -1) {
        WSASetLastError(map_error_to_winsock(errno));
        return SOCKET_ERROR;
    }

    if (lpNumberOfBytesSent) {
        *lpNumberOfBytesSent = ret;
    }
    return 0;
}
