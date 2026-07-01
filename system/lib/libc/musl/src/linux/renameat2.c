#define _GNU_SOURCE
#include <stdio.h>
#include "syscall.h"

int renameat2(int oldfd, const char *old, int newfd, const char *new, unsigned flags)
{
#ifdef SYS_renameat
	if (!flags) return syscall(SYS_renameat, oldfd, old, newfd, new);
#endif
	return syscall(SYS_renameat2, oldfd, old, newfd, new, flags);
}
