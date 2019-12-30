#include <dirent.h>
#include <assert.h>
#include <errno.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>

DIR *opendir (const char *dirname)
{
    if (dirname == NULL) {
        errno = EFAULT;
        return NULL;
    }

    size_t dirlen = strlen(dirname);
    if (dirlen > MAX_PATH-2) {
        errno = ENAMETOOLONG;
        return NULL;
    }

    if (dirlen == 0) {
        errno = ENOENT;
        return NULL;
    }

    // FIXME: check if it's really a directory, return ENOTDIR

    DIR *dir = malloc(sizeof(DIR));
    if (!dir) {
        errno = ENOMEM;
        return NULL;
    }

    strcpy(dir->path, dirname);
    if (dir->path[dirlen-1] != '\\') {
        strcat(dir->path, "\\*");
    } else {
        strcat(dir->path, "*");
    }

    dir->handle = INVALID_HANDLE_VALUE;
    dir->ready = false;
    return dir;
}

int closedir (DIR *dir)
{
    assert(dir != NULL);

    int result;

    if (dir->handle != INVALID_HANDLE_VALUE) {
        if (!FindClose(dir->handle)) {
            // FIXME: Needs the PDCLib-PR to work!
            //_PDCLIB_w32errno();
            result = -1;
        } else {
            result = 0;
        }
    } else {
        result = -1;
        errno = EFAULT;
    }

    free(dir);

    return result;
}

struct dirent *readdir (DIR * dir)
{
    assert(dir != NULL);

    WIN32_FIND_DATAA findFileData;

    if (!dir->ready) {
        dir->ready = true;
        dir->handle = FindFirstFileA(dir->path, &findFileData);
        if (dir->handle == INVALID_HANDLE_VALUE) {
            // FIXME: Needs the PDCLib-PR to work!
            //_PDCLIB_w32errno();
            return NULL;
        }
    } else {
        int rval = FindNextFileA(dir->handle, &findFileData);
        if (rval == 0 && GetLastError() == ERROR_NO_MORE_FILES) {
            // FIXME: Needs the PDCLib-PR to work!
            //_PDCLIB_w32errno();
            return NULL;
        }
    }

    memset(&dir->dir_ent, 0, sizeof(struct dirent));
    strcpy(dir->dir_ent.d_name, findFileData.cFileName);
    assert(strlen(findFileData.cFileName) <= 255);
    dir->dir_ent.d_namlen = strlen(findFileData.cFileName);

    return &dir->dir_ent;
}

void rewinddir (DIR * dir)
{
    assert(dir != NULL);

    int result;
    if (dir->handle != INVALID_HANDLE_VALUE) {
        if (!FindClose(dir->handle)) {
            // FIXME: Needs the PDCLib-PR to work!
            //_PDCLIB_w32errno();
            result = -1;
        } else {
            result = 0;
        }
    } else {
        result = -1;
        errno = EFAULT;
    }

    dir->handle = INVALID_HANDLE_VALUE;
    dir->ready = false;
}
