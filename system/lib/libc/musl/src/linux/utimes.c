#include <sys/time.h>
#include "fcntl.h"
#include "syscall.h"

int __futimesat(int, const char *, const struct timeval [2]);

int utimes(const char *path, const struct timeval times[2])
{
	return __futimesat(AT_FDCWD, path, times);
}
