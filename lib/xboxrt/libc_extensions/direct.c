#include <direct.h>
#include <windows.h>

int _mkdir (const char *dirname)
{
    BOOL success = CreateDirectoryA(dirname, NULL);

    if (!success) {
        // FIXME: Depends on PDCLib PR
        //_PDCLIB_w32errno();
        return -1;
    }

    return 0;
}