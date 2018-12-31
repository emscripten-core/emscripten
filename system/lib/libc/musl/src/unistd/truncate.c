#include <unistd.h>
#include "syscall.h"
#include "libc.h"

int truncate(const char *path, off_t length)
{
	return syscall(SYS_truncate, path, __SYSCALL_LL_O(length));
}

LFS64(truncate);
