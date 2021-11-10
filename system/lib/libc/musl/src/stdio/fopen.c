#include "stdio_impl.h"
#include <fcntl.h>
#include <string.h>
#include <errno.h>

FILE *fopen(const char *restrict filename, const char *restrict mode)
{
	FILE *f;
	int fd;
	int flags;

	/* Check for valid initial mode character */
	if (!strchr("rwa", *mode)) {
		errno = EINVAL;
		return 0;
	}

	/* Compute the flags to pass to open() */
	flags = __fmodeflags(mode);

	fd = sys_open(filename, flags, 0666);
	if (fd < 0) return 0;
#ifndef __EMSCRIPTEN__ // CLOEXEC makes no sense for a single process
	if (flags & O_CLOEXEC)
		__syscall(SYS_fcntl, fd, F_SETFD, FD_CLOEXEC);
#endif

	f = __fdopen(fd, mode);
	if (f) return f;

#ifdef __EMSCRIPTEN__
	__wasi_fd_close(fd);
#else
	__syscall(SYS_close, fd);
#endif
	return 0;
}

weak_alias(fopen, fopen64);
