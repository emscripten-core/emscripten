#define _GNU_SOURCE
#include <sys/uio.h>
#include <unistd.h>
#include <fcntl.h>
#include "syscall.h"

ssize_t pwritev(int fd, const struct iovec *iov, int count, off_t ofs)
{
	if (ofs == -1) ofs--;
	int r = __syscall_cp(SYS_pwritev2, fd, iov, count,
		(long)(ofs), (long)(ofs>>32), RWF_NOAPPEND);
	if (r != -EOPNOTSUPP && r != -ENOSYS)
		return __syscall_ret(r);
	if (fcntl(fd, F_GETFL) & O_APPEND)
		return __syscall_ret(-EOPNOTSUPP);
	return syscall_cp(SYS_pwritev, fd, iov, count,
		(long)(ofs), (long)(ofs>>32));
}
