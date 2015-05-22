#include <sys/epoll.h>
#include <signal.h>
#include "syscall.h"

int epoll_create(int size)
{
	return syscall(SYS_epoll_create, size);
}

int epoll_create1(int flags)
{
	return syscall(SYS_epoll_create1, flags);
}

int epoll_ctl(int fd, int op, int fd2, struct epoll_event *ev)
{
	return syscall(SYS_epoll_ctl, fd, op, fd2, ev);
}

int epoll_pwait(int fd, struct epoll_event *ev, int cnt, int to, const sigset_t *sigs)
{
	return syscall(SYS_epoll_pwait, fd, ev, cnt, to, sigs, _NSIG/8);
}

int epoll_wait(int fd, struct epoll_event *ev, int cnt, int to)
{
	return syscall(SYS_epoll_wait, fd, ev, cnt, to);
}
