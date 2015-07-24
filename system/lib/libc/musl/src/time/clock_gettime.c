#include <time.h>
#include <errno.h>
#include <stdint.h>
#include "syscall.h"
#include "libc.h"

static int sc_clock_gettime(clockid_t clk, struct timespec *ts)
{
	int r = __syscall(SYS_clock_gettime, clk, ts);
	if (!r) return r;
	if (r == -ENOSYS) {
		if (clk == CLOCK_REALTIME) {
			__syscall(SYS_gettimeofday, ts, 0);
			ts->tv_nsec = (int)ts->tv_nsec * 1000;
			return 0;
		}
		r = -EINVAL;
	}
	errno = -r;
	return -1;
}

weak_alias(sc_clock_gettime, __vdso_clock_gettime);

int (*__cgt)(clockid_t, struct timespec *) = __vdso_clock_gettime;

int __clock_gettime(clockid_t clk, struct timespec *ts)
{
	/* Conditional is to make this work prior to dynamic linking */
	return __cgt ? __cgt(clk, ts) : sc_clock_gettime(clk, ts);
}

weak_alias(__clock_gettime, clock_gettime);
