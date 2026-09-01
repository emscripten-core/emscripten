#include <dirent.h>
#include <limits.h>
#include <errno.h>
#include "syscall.h"

ssize_t posix_getdents(int fd, void *buf, size_t len, int flags)
{
	if (flags) return __syscall_ret(-EOPNOTSUPP);
	if (len>INT_MAX) len = INT_MAX;
	return syscall(SYS_getdents, fd, buf, len);
}
