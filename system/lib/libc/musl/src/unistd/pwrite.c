#define _GNU_SOURCE
#include <unistd.h>
#include <sys/uio.h>
#include <fcntl.h>
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
	if (ofs == -1) ofs--;
	int r = __syscall_cp(SYS_pwritev2, fd,
		(&(struct iovec){ .iov_base = (void *)buf, .iov_len = size }),
		1, (long)(ofs), (long)(ofs>>32), RWF_NOAPPEND);
	if (r != -EOPNOTSUPP && r != -ENOSYS)
		return __syscall_ret(r);
	if (fcntl(fd, F_GETFL) & O_APPEND)
		return __syscall_ret(-EOPNOTSUPP);
	return syscall_cp(SYS_pwrite, fd, buf, size, __SYSCALL_LL_PRW(ofs));
#endif
}
