#ifndef _XBOXRT_DIRENT_H_
#define _XBOXRT_DIRENT_H_

#include <stdbool.h>
#include <sys/types.h>

typedef struct _DIR
{
    char path[260];
    void *handle;
    bool ready;
    struct dirent dir_ent;
} DIR;

DIR *opendir (const char *dirname);
int closedir (DIR *dir);
struct dirent *readdir (DIR * dir);
void rewinddir (DIR * dir);

#endif
