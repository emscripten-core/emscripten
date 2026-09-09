#define _GNU_SOURCE
#include <sys/socket.h>
#include <errno.h>
#include <fcntl.h>
#include "syscall.h"
#ifdef __EMSCRIPTEN_PTHREADS__
#include "emscripten_fd_wait.h"
#endif

int accept4(int fd, struct sockaddr *restrict addr, socklen_t *restrict len, int flg)
{
	if (!flg) return accept(fd, addr, len);
#ifdef __EMSCRIPTEN_PTHREADS__
	int ret;
	for (;;) {
		long r = __socketcall_cp(accept4, fd, addr, len, flg, 0, 0);
		if (r != -EAGAIN || !__emscripten_sock_can_wait(fd, 0)
		    || _emscripten_fd_wait(fd, POLLIN)) {
			ret = __syscall_ret(r);
			break;
		}
	}
#else
	int ret = socketcall_cp(accept4, fd, addr, len, flg, 0, 0);
#endif
	if (ret>=0 || (errno != ENOSYS && errno != EINVAL)) return ret;
	if (flg & ~(SOCK_CLOEXEC|SOCK_NONBLOCK)) {
		errno = EINVAL;
		return -1;
	}
	ret = accept(fd, addr, len);
	if (ret<0) return ret;
	if (flg & SOCK_CLOEXEC)
		__syscall(SYS_fcntl, ret, F_SETFD, FD_CLOEXEC);
	if (flg & SOCK_NONBLOCK)
		__syscall(SYS_fcntl, ret, F_SETFL, O_NONBLOCK);
	return ret;
}
