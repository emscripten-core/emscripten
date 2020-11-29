#include <unistd.h>
#include "syscall.h"

ssize_t write(int fd, const void *buf, size_t count)
{
#if __EMSCRIPTEN__
	__wasi_ciovec_t iov = {
		.buf = buf,
		.buf_len = count
	};
	size_t num;
	if (__wasi_syscall_ret(__wasi_fd_write(fd, &iov, 1, &num))) {
		return -1;
	}
	return num;
#else
	return syscall_cp(SYS_write, fd, buf, count);
#endif
}
