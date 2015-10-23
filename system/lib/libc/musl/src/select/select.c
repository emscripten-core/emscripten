#include <sys/select.h>
#include "syscall.h"
#include "libc.h"

int select(int n, fd_set *restrict rfds, fd_set *restrict wfds, fd_set *restrict efds, struct timeval *restrict tv)
{
	return syscall_cp(SYS_select, n, rfds, wfds, efds, tv);
}
