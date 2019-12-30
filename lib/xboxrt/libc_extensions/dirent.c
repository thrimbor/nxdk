#include <dirent.h>
#include <assert.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>

DIR *opendir (const char *dirname)
{
    if (dirname == NULL) {
        //errno = EFAULT;
        return NULL;
    }

    size_t dirlen = strlen(dirname);
    if (dirlen > MAX_PATH-2) {
        //errno = ENAMETOOLONG;
        return NULL;
    }

    if (dirlen == 0) {
        //errno = ENOENT;
        return NULL;
    }

    // FIXME: check if it's really a directory

    DIR *dir = malloc(sizeof(DIR));
    if (!dir) {
        // FIXME: set errno
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

    if (dir->handle != INVALID_HANDLE_VALUE) {
        FindClose(dir->handle);
        // FIXME: errno?
    }

    free(dir);

    // FIXME what do we return?
    return 1;
}

struct dirent *readdir (DIR * dir)
{
    assert(dir != NULL);

    WIN32_FIND_DATAA findFileData;

    if (!dir->ready) {
        dir->ready = true;
        dir->handle = FindFirstFileA(dir->path, &findFileData);
        if (dir->handle == INVALID_HANDLE_VALUE) {
            // FIXME: errno
            return NULL;
        }
    } else {
        int rval = FindNextFileA(dir->handle, &findFileData);
        if (rval == 0 && GetLastError() == ERROR_NO_MORE_FILES) {
            //FIXME: errno
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

    if (dir->handle != INVALID_HANDLE_VALUE) {
        FindClose(dir->handle);
        // FIXME: errno?
    }

    dir->handle = INVALID_HANDLE_VALUE;
    dir->ready = false;
}
