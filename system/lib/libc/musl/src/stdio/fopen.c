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

	fd = syscall_cp(SYS_open, filename, flags|O_LARGEFILE, 0666);
	if (fd < 0) return 0;

	f = __fdopen(fd, mode);
	if (f) return f;

	__syscall(SYS_close, fd);
	return 0;
}

LFS64(fopen);
