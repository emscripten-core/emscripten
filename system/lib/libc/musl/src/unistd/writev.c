#include <sys/uio.h>
#include "syscall.h"
#include "libc.h"
#include <wasi/wasi.h>

ssize_t writev(int fd, const struct iovec *iov, int count)
{
#if __EMSCRIPTEN__
		size_t num;
		__wasi_errno_t error = __wasi_fd_write(fd, (struct __wasi_ciovec_t*)iov, count, &num);
		if (error) {
				num = -1;
		}
		return num;
#else
		return syscall_cp(SYS_writev, fd, iov, count);
#endif
}
