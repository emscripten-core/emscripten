#ifdef __EMSCRIPTEN__
#include <stropts.h>
#endif
#include <sys/stat.h>
#include <errno.h>
#include <fcntl.h>
#include "syscall.h"
#include "libc.h"

void __procfdname(char *, unsigned);

int fstat(int fd, struct stat *st)
{
	int ret = __syscall(SYS_fstat, fd, st);
#if __EMSCRIPTEN__
	if (ret != -EBADF || !__wasi_fd_is_valid(fd))
		return __syscall_ret(ret);
#else
	if (ret != -EBADF || __syscall(SYS_fcntl, fd, F_GETFD) < 0)
		return __syscall_ret(ret);
#endif

	char buf[15+3*sizeof(int)];
	__procfdname(buf, fd);
#ifdef SYS_stat
	return syscall(SYS_stat, buf, st);
#else
	return syscall(SYS_fstatat, AT_FDCWD, buf, st, 0);
#endif
}

LFS64(fstat);
