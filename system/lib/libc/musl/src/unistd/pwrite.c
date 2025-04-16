#include <unistd.h>
#include "syscall.h"

ssize_t pwrite(int fd, const void *buf, size_t size, off_t ofs)
{
#if __EMSCRIPTEN__
	__wasi_ciovec_t iov = {
		.buf = buf,
		.buf_len = size
	};
	size_t num;
	if (__wasi_syscall_ret(__wasi_fd_pwrite(fd, &iov, 1, ofs, &num))) {
		return -1;
	}
	return num;
#else
	return syscall_cp(SYS_pwrite, fd, buf, size, __SYSCALL_LL_PRW(ofs));
#endif
}
