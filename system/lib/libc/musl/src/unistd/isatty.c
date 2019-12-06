#ifdef __EMSCRIPTEN__
#include <errno.h>
#endif
#include <unistd.h>
#include <sys/ioctl.h>
#include "syscall.h"

int isatty(int fd)
{
#ifdef __EMSCRIPTEN__
	__wasi_fdstat_t statbuf;
	int err = __wasi_fd_fdstat_get(fd, &statbuf);
	if (err != 0) {
		errno = err;
		return 0;
	}

	// All character devices are terminals (other things a Linux system would
	// assume is a character device, like the mouse, we have special APIs for).
	if (statbuf.fs_filetype != __WASI_FILETYPE_CHARACTER_DEVICE) {
		errno = __WASI_ERRNO_NOTTY;
		return 0;
	}

	return 1;
#else
	struct winsize wsz;
	return !syscall(SYS_ioctl, fd, TIOCGWINSZ, &wsz);
#endif
}
