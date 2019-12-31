#include <io.h>
#include <fcntl.h>
#include <assert.h>
#include <errno.h>
#include <windows.h>

#define MAX_PIO_FILES 32
HANDLE pio_fds[MAX_PIO_FILES];

CRITICAL_SECTION pioFileCs;

__attribute__((constructor)) void _init_fds (void)
{
    InitializeCriticalSection(&pioFileCs);
}

int _reserve_fd (HANDLE handle)
{
    int fd = -1;

    RtlEnterCriticalSection(&pioFileCs);
    for (int i = 0; i < MAX_PIO_FILES; i++) {
        if (pio_fds[i] != INVALID_HANDLE_VALUE) {
            pio_fds[i] = handle;
            fd = i;
            break;
        }
    }
    RtlLeaveCriticalSection(&pioFileCs);

    return fd;
}


static void _free_fd (int fd)
{
    EnterCriticalSection(&pioFileCs);
    assert(pio_fds[fd] != INVALID_HANDLE_VALUE);
    pio_fds[fd] = INVALID_HANDLE_VALUE;
    LeaveCriticalSection(&pioFileCs);
}

int open (const char *filename, int oflag, ...)
{
    DWORD access = 0;
    DWORD creation;
    DWORD share;
    DWORD attrib;

    if (oflag & _O_RDONLY) access |= GENERIC_READ;
    if (oflag & _O_WRONLY) access |= GENERIC_WRITE;
    if (oflag & _O_RDWR) access |= GENERIC_WRITE | GENERIC_READ;

    if (oflag & _O_CREAT) {
        if (oflag & _O_EXCL) creation = CREATE_NEW;
        else if (oflag & _O_TRUNC) creation = CREATE_ALWAYS;
        else creation = OPEN_ALWAYS;
    } else {
        if (oflag & _O_TRUNC) creation = TRUNCATE_EXISTING;
        else creation = OPEN_EXISTING;
    }

    share = FILE_SHARE_READ | FILE_SHARE_WRITE;

    if (oflag & _O_TEMPORARY) {
        attrib |= FILE_FLAG_DELETE_ON_CLOSE;
        access |= DELETE;
        share |= FILE_SHARE_DELETE;
    }

    HANDLE handle = CreateFileA(filename, access, share, NULL, creation, attrib, 0);
    if (handle == INVALID_HANDLE_VALUE) {
        // FIXME: Needs the PDCLib-PR to work!
        //_PDCLIB_w32errno();
        return -1;

    }

    int fd = _reserve_fd(handle);
    if (fd == -1) {
        errno = EMFILE;
        CloseHandle(handle);
    }

    return fd;
}

int close (int fd)
{
    assert(pio_fds[fd] != INVALID_HANDLE_VALUE);

    BOOL ret = CloseHandle(pio_fds[fd]);
    _free_fd(fd);
    if (!ret) {
        // FIXME: Needs the PDCLib-PR to work!
        //_PDCLIB_w32errno();
        return -1;
    }

    return 0;
}

int read (int fd, void *buf, unsigned int count)
{
    HANDLE handle = pio_fds[fd];

    assert(handle != INVALID_HANDLE_VALUE);

    DWORD bytesRead = 0;
    BOOL ret = ReadFile(handle, buf, count, &bytesRead, NULL);

    if (!ret) {
        // FIXME: Needs the PDCLib-PR to work!
        //_PDCLIB_w32errno();
        return -1;
    }

    return bytesRead;
}

int write (int fd, const void *buf, unsigned int count)
{
    HANDLE handle = pio_fds[fd];

    assert(handle != INVALID_HANDLE_VALUE);

    DWORD bytesWritten = 0;
    BOOL ret = WriteFile(handle, buf, count, &bytesWritten, NULL);

    if (!ret) {
        // FIXME: Needs the PDCLib-PR to work!
        //_PDCLIB_w32errno();
        return -1;
    }

    return bytesWritten;
}
