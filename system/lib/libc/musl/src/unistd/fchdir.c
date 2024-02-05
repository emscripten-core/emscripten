#ifdef __EMSCRIPTEN__
#include <stropts.h>
#endif
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include "syscall.h"

int fchdir(int fd)
{
	int ret = __syscall(SYS_fchdir, fd);
#if __EMSCRIPTEN__
	if (ret != -EBADF || !__wasi_fd_is_valid(fd))
		return __syscall_ret(ret);
#else
	if (ret != -EBADF || __syscall(SYS_fcntl, fd, F_GETFD) < 0)
		return __syscall_ret(ret);
#endif

	char buf[15+3*sizeof(int)];
	__procfdname(buf, fd);
	return syscall(SYS_chdir, buf);
}
