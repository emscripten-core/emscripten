#include <unistd.h>
#include "syscall.h"
#include "libc.h"

ssize_t write(int fd, const void *buf, size_t count)
{
#if __EMSCRIPTEN__
	__wasi_ciovec_t iov = {
		.buf = buf,
		.buf_len = count
	};
	size_t num;
	__wasi_errno_t error =
			__wasi_fd_write(fd, &iov, 1, &num);
	if (error != __WASI_ERRNO_SUCCESS) {
		return __wasi_syscall_ret(error);
	}
	return num;
#else
	return syscall_cp(SYS_write, fd, buf, count);
#endif
}
