#include <sys/stat.h>
#include "libc.h"

int __fxstatat(int ver, int fd, const char *path, struct stat *buf, int flag)
{
	return fstatat(fd, path, buf, flag);
}

LFS64(__fxstatat);
