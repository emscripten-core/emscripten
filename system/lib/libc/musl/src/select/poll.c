#include <poll.h>
#include <time.h>
#include <signal.h>
#include "syscall.h"

int poll(struct pollfd *fds, nfds_t n, int timeout)
{
#ifdef __EMSCRIPTEN__
	// A zero timeout is an instantaneous probe: route it through a plain
	// import that never suspends. Under JSPI, __syscall_poll is a suspending
	// import and so may only be called from a stack entered through a
	// promising export — a requirement a readiness probe must not carry
	// (e.g. probes from event-loop callbacks).
	if (timeout == 0) {
		return __syscall_ret(__syscall_poll_nonblocking((intptr_t)fds, n));
	}
#endif
#ifdef SYS_poll
	return syscall_cp(SYS_poll, fds, n, timeout);
#else
#if SYS_ppoll_time64 == SYS_ppoll
	typedef long long ppoll_ts_t[2];
#else
	typedef long ppoll_ts_t[2];
#endif
	return syscall_cp(SYS_ppoll, fds, n, timeout>=0 ?
		((ppoll_ts_t){ timeout/1000, timeout%1000*1000000 }) : 0,
		0, _NSIG/8);
#endif
}
