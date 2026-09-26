#include <sys/epoll.h>
#include <signal.h>
#include <errno.h>
#include "syscall.h"

int epoll_create(int size)
{
	if (size<=0) return __syscall_ret(-EINVAL);
	return epoll_create1(0);
}

int epoll_create1(int flags)
{
	int r = __syscall(SYS_epoll_create1, flags);
#ifdef SYS_epoll_create
	if (r==-ENOSYS && !flags) r = __syscall(SYS_epoll_create, 1);
#endif
	return __syscall_ret(r);
}

int epoll_ctl(int fd, int op, int fd2, struct epoll_event *ev)
{
	return syscall(SYS_epoll_ctl, fd, op, fd2, ev);
}

int epoll_pwait(int fd, struct epoll_event *ev, int cnt, int to, const sigset_t *sigs)
{
#ifdef __EMSCRIPTEN__
	// A zero timeout is an instantaneous probe: route it through a plain
	// import that never suspends. Under JSPI, __syscall_epoll_pwait is a
	// suspending import and so may only be called from a stack entered through
	// a promising export — a requirement a readiness probe must not carry
	// (e.g. probes from event-loop callbacks). Mirrors poll() above.
	if (to == 0) {
		return __syscall_ret(__syscall_epoll_pwait_nonblocking(fd, ev, cnt));
	}
#endif
	int r = __syscall_cp(SYS_epoll_pwait, fd, ev, cnt, to, sigs, _NSIG/8);
#ifdef SYS_epoll_wait
	if (r==-ENOSYS && !sigs) r = __syscall_cp(SYS_epoll_wait, fd, ev, cnt, to);
#endif
	return __syscall_ret(r);
}

int epoll_wait(int fd, struct epoll_event *ev, int cnt, int to)
{
	return epoll_pwait(fd, ev, cnt, to, 0);
}
