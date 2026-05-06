#include <sys/uio.h>
#include "syscall.h"

ssize_t readv(int fd, const struct iovec *iov, int count)
{
#if __EMSCRIPTEN__
	size_t num;
	if (__wasi_syscall_ret(__wasi_fd_read(fd, (struct __wasi_iovec_t*)iov, count, &num))) {
		num = -1;
	}
	return num;
#else
	return syscall_cp(SYS_readv, fd, iov, count);
#endif
}
