#include <utime.h>
#include <sys/time.h>
#include "syscall.h"

int utime(const char *path, const struct utimbuf *times)
{
	if (times) {
		struct timeval tv[2] = {
			{ .tv_sec = times->actime },
			{ .tv_sec = times->modtime } };
		return syscall(SYS_utimes, path, tv);
	}
	return syscall(SYS_utimes, path, 0);
}
