#define _BSD_SOURCE
#include <poll.h>
#include <signal.h>
#include <errno.h>
#include "syscall.h"

#define IS32BIT(x) !((x)+0x80000000ULL>>32)
#define CLAMP(x) (int)(IS32BIT(x) ? (x) : 0x7fffffffU+((0ULL+(x))>>63))

int ppoll(struct pollfd *fds, nfds_t n, const struct timespec *to, const sigset_t *mask)
{
#ifdef __EMSCRIPTEN__
	// Emscripten does not support true async signals so we just implement ppoll
	// in terms of poll here in userspace.
	int timeout = (to == NULL) ? -1 : (to->tv_sec * 1000 + to->tv_nsec / 1000000);
	sigset_t origmask;
	pthread_sigmask(SIG_SETMASK, mask, &origmask);
	int rtn = poll(fds, n, timeout);
	pthread_sigmask(SIG_SETMASK, &origmask, NULL);
	return rtn;
#else
	time_t s = to ? to->tv_sec : 0;
	long ns = to ? to->tv_nsec : 0;
#ifdef SYS_ppoll_time64
	int r = -ENOSYS;
	if (SYS_ppoll == SYS_ppoll_time64 || !IS32BIT(s))
		r = __syscall_cp(SYS_ppoll_time64, fds, n,
			to ? ((long long[]){s, ns}) : 0,
			mask, _NSIG/8);
	if (SYS_ppoll == SYS_ppoll_time64 || r != -ENOSYS)
		return __syscall_ret(r);
	s = CLAMP(s);
#endif
	return syscall_cp(SYS_ppoll, fds, n,
		to ? ((long[]){s, ns}) : 0, mask, _NSIG/8);
#endif
}
