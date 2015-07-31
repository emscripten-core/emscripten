#define _GNU_SOURCE
#include <sys/time.h>
#include "syscall.h"

#ifdef SYS_futimesat
int futimesat(int dirfd, const char *pathname, const struct timeval times[2])
{
	return syscall(SYS_futimesat, dirfd, pathname, times);
}
#endif
