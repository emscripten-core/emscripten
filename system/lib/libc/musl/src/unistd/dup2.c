#include <unistd.h>
#include <errno.h>
#include "syscall.h"

int dup2(int old, int new)
{
	int r;
	while ((r=__syscall(SYS_dup2, old, new))==-EBUSY);
	return __syscall_ret(r);
}
