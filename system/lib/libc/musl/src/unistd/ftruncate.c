#include <unistd.h>
#include "syscall.h"
#include "libc.h"

int ftruncate(int fd, off_t length)
{
	return syscall(SYS_ftruncate, fd, __SYSCALL_LL_O(length));
}

LFS64(ftruncate);
