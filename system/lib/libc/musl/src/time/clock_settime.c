#include <time.h>
#include "syscall.h"
#ifdef __EMSCRIPTEN__
#include <errno.h>
#endif

int clock_settime(clockid_t clk, const struct timespec *ts)
{
#ifdef __EMSCRIPTEN__
	// JS and wasm VMs do not allow setting the time.
	errno = EPERM;
	return -1;
#else
	return syscall(SYS_clock_settime, clk, ts);
#endif
}
