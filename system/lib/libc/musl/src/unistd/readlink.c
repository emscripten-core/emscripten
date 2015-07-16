#include <unistd.h>
#include "syscall.h"

ssize_t readlink(const char *restrict path, char *restrict buf, size_t bufsize)
{
	return syscall(SYS_readlink, path, buf, bufsize);
}
