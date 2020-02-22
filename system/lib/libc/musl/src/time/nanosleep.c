#include <time.h>
#include "syscall.h"
#include "libc.h"
#if __EMSCRIPTEN__
#include <errno.h>
#endif

int nanosleep(const struct timespec *req, struct timespec *rem)
{
#if __EMSCRIPTEN__
	// Without threads support, nanosleep cannot actually work except to do a
	// silly busy-wait. FIXME with pthreads in standalone mode
	errno = ENOSYS;
	return -1;
#else
	return syscall_cp(SYS_nanosleep, req, rem);
#endif
}
