#include <sys/select.h>
#include <signal.h>
#include <stdint.h>
#include <errno.h>
#include "syscall.h"

#define IS32BIT(x) !((x)+0x80000000ULL>>32)
#define CLAMP(x) (int)(IS32BIT(x) ? (x) : 0x7fffffffU+((0ULL+(x))>>63))

#ifdef __EMSCRIPTEN__
#include <stdlib.h>
#include <poll.h>
static int emscripten_select(int nfds, fd_set *readfds, fd_set *writefds, fd_set *exceptfds, struct timeval *tv);
#endif

int select(int n, fd_set *restrict rfds, fd_set *restrict wfds, fd_set *restrict efds, struct timeval *restrict tv)
{
	time_t s = tv ? tv->tv_sec : 0;
	suseconds_t us = tv ? tv->tv_usec : 0;
	long ns;
	const time_t max_time = (1ULL<<8*sizeof(time_t)-1)-1;

	if (s<0 || us<0) return __syscall_ret(-EINVAL);
#ifdef __EMSCRIPTEN__
	return emscripten_select(n, rfds, wfds, efds, tv);
#else
	if (us/1000000 > max_time - s) {
		s = max_time;
		us = 999999;
		ns = 999999999;
	} else {
		s += us/1000000;
		us %= 1000000;
		ns = us*1000;
	}
#ifdef SYS_pselect6_time64
	int r = -ENOSYS;
	if (SYS_pselect6 == SYS_pselect6_time64 || !IS32BIT(s))
		r = __syscall_cp(SYS_pselect6_time64, n, rfds, wfds, efds,
			tv ? ((long long[]){s, ns}) : 0,
			((syscall_arg_t[]){ 0, _NSIG/8 }));
	if (SYS_pselect6 == SYS_pselect6_time64 || r!=-ENOSYS)
		return __syscall_ret(r);
	s = CLAMP(s);
#endif
#ifdef SYS_select
	return syscall_cp(SYS_select, n, rfds, wfds, efds,
		tv ? ((long[]){s, us}) : 0);
#else
	return syscall_cp(SYS_pselect6, n, rfds, wfds, efds,
		tv ? ((long[]){s, ns}) : 0, ((syscall_arg_t[]){ 0, _NSIG/8 }));
#endif
#endif
}

#ifdef __EMSCRIPTEN__
static int emscripten_select(int nfds, fd_set *readfds, fd_set *writefds, fd_set *exceptfds, struct timeval *tv)
{
	// Implement select in terms of `poll()`

	// Part 1: convert select arguments into poll arguments

	time_t s = tv ? tv->tv_sec : 0;
	suseconds_t us = tv ? tv->tv_usec : 0;
	int timeout =  tv ? s * 1000 + (us / 1000) : -1;
	int n = 0;
	struct pollfd* fds = (struct pollfd*)calloc(nfds, sizeof(struct pollfd));

	for (int i = 0; i < nfds; i++) {
		if (readfds && FD_ISSET(i, readfds)) {
			fds[n].events |= POLLIN;
		}
		if (writefds && FD_ISSET(i, writefds)) {
			fds[n].events |= POLLOUT;
		}
		if (exceptfds && FD_ISSET(i, exceptfds)) {
			fds[n].events |= POLLPRI;
		}
		if (fds[n].events) {
			fds[n].fd = i;
			n++;
		}
	}

	int rtn = __syscall_poll((intptr_t)fds, n, timeout);
	if (rtn < 0) {
		free(fds);
		return -1;
	}

	// Part 2: Translate the result of poll into the results of select();

	if (readfds)	 FD_ZERO(readfds);
	if (writefds)  FD_ZERO(writefds);
	if (exceptfds) FD_ZERO(exceptfds);

	int count = 0;

	if (rtn > 0) {
		for (int i = 0; i < n; i++) {
			int fd = fds[i].fd;
			short revents = fds[i].revents;
			if (revents) {
				// Map POLLIN to readfds
				// POLLHUP/POLLERR usually count as readable (EOF or Error)
				if (readfds && (revents & POLLIN || revents & POLLHUP || revents & POLLERR)) {
					FD_SET(fd, readfds);
					count++;
				}
				// Map POLLOUT to writefds
				// POLLERR usually counts as writable (so write fails immediately)
				if (writefds && (revents & POLLOUT || revents & POLLERR)) {
					FD_SET(fd, writefds);
					count++;
				}
				// Map POLLPRI to exceptfds
				if (exceptfds && (revents & POLLPRI)) {
					FD_SET(fd, exceptfds);
					count++;
				}
			}
		}
	}

	free(fds);
	return count;
}
#endif
