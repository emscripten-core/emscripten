#define _BSD_SOURCE
#ifdef __EMSCRIPTEN__
#include <stropts.h>
#endif
#include <sys/stat.h>
#include <errno.h>
#include <fcntl.h>
#include "syscall.h"

int fstat(int fd, struct stat *st)
{
#ifdef __EMSCRIPTEN__
	if (!__wasi_fd_is_valid(fd)) return __syscall_ret(-EBADF);
#else
	if (fd<0) return __syscall_ret(-EBADF);
#endif

	return fstatat(fd, "", st, AT_EMPTY_PATH);
}

#if !_REDIR_TIME64
weak_alias(fstat, fstat64);
#endif
