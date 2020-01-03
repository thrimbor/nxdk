#ifndef _XBOXRT_IO_H_
#define _XBOXRT_IO_H_

#define _open open
int open (const char *filename, int oflag, ...);

#define _read read
int read (int fd, void *buf, unsigned int count);

#define _write write
int write (int fd, const void *buffer, unsigned int count);

#define _close close
int close (int fd);

#endif
