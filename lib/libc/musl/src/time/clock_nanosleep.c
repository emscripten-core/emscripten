#include <time.h>
#include "syscall.h"
#include "libc.h"

int clock_nanosleep(clockid_t clk, int flags, const struct timespec *req, struct timespec *rem)
{
	return -__syscall_cp(SYS_clock_nanosleep, clk, flags, req, rem);
}
