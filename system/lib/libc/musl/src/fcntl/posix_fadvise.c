#include <fcntl.h>
#include "syscall.h"
#include "libc.h"

int posix_fadvise(int fd, off_t base, off_t len, int advice)
{
	return -(__syscall)(SYS_fadvise, fd, __SYSCALL_LL_O(base),
		__SYSCALL_LL_E(len), advice);
}

LFS64(posix_fadvise);
