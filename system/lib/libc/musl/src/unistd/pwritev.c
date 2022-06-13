#define _BSD_SOURCE
#include <sys/uio.h>
#include <unistd.h>
#include "syscall.h"

ssize_t pwritev(int fd, const struct iovec *iov, int count, off_t ofs)
{
#if __EMSCRIPTEN__
	size_t num;
	if (__wasi_syscall_ret(__wasi_fd_pwrite(fd, (struct __wasi_ciovec_t*)iov, count, ofs, &num))) {
		return -1;
	}
	return num;
#else
	return syscall_cp(SYS_pwritev, fd, iov, count,
		(long)(ofs), (long)(ofs>>32));
#endif
}

weak_alias(pwritev, pwritev64);
