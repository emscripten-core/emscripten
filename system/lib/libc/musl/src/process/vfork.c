#define _GNU_SOURCE
#include <unistd.h>
#include "syscall.h"
#include "libc.h"

pid_t __vfork(void)
{
	/* vfork syscall cannot be made from C code */
	return syscall(SYS_fork);
}

weak_alias(__vfork, vfork);
