#include "futex.h"
#include "syscall.h"

int __futex(volatile int *addr, int op, int val, void *ts)
{
	return syscall(SYS_futex, addr, op, val, ts);
}
