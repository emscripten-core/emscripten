#include <sys/uio.h>
#include "syscall.h"
#if __EMSCRIPTEN__
#include <wasi/api.h>
#endif

ssize_t writev(int fd, const struct iovec *iov, int count)
{
#if __EMSCRIPTEN__
	size_t num;
	if (__wasi_syscall_ret(__wasi_fd_write(fd, (struct __wasi_ciovec_t*)iov, count, &num))) {
		return -1;
	}
	return num;
#else
	return syscall_cp(SYS_writev, fd, iov, count);
#endif
}
