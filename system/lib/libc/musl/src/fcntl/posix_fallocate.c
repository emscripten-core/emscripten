#include <fcntl.h>
#include "syscall.h"
#include "libc.h"

int posix_fallocate(int fd, off_t base, off_t len)
{
	return -__syscall(SYS_fallocate, fd, 0, __SYSCALL_LL_E(base),
		__SYSCALL_LL_E(len));
}

LFS64(posix_fallocate);
