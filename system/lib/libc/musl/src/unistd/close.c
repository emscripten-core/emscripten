#include <unistd.h>
#include <errno.h>
#include "syscall.h"
#include "libc.h"

int close(int fd)
{
	int r = __syscall_cp(SYS_close, fd);
	if (r == -EINTR) r = -EINPROGRESS;
	return __syscall_ret(r);
}
