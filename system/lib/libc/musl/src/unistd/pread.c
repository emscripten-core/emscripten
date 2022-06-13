#include <unistd.h>
#include "syscall.h"

ssize_t pread(int fd, void *buf, size_t size, off_t ofs)
{
#if __EMSCRIPTEN__
	__wasi_iovec_t iov = {
		.buf = buf,
		.buf_len = size
	};
	size_t num;
	if (__wasi_syscall_ret(__wasi_fd_pread(fd, &iov, 1, ofs, &num))) {
		return -1;
	}
	return num;
#else
	return syscall_cp(SYS_pread, fd, buf, size, __SYSCALL_LL_PRW(ofs));
#endif
}

weak_alias(pread, pread64);
