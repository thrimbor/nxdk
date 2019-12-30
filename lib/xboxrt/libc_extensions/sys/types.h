#ifndef _XBOXRT_TYPES_H_
#define _XBOXRT_TYPES_H_

#define _DIRENT_HAVE_D_NAMLEN
struct dirent
{
    char d_name[256];
    int d_namlen;
};

#endif
