#include <sys/socket.h>
#include "syscall.h"
#ifdef __EMSCRIPTEN_PTHREADS__
#include "emscripten_fd_wait.h"
#endif

ssize_t recvfrom(int fd, void *restrict buf, size_t len, int flags, struct sockaddr *restrict addr, socklen_t *restrict alen)
{
#ifdef __EMSCRIPTEN_PTHREADS__
	for (;;) {
		long r = __socketcall_cp(recvfrom, fd, buf, len, flags, addr, alen);
		if (r != -EAGAIN || !__emscripten_sock_can_wait(fd, flags & MSG_DONTWAIT)
		    || _emscripten_fd_wait(fd, POLLIN))
			return __syscall_ret(r);
	}
#else
	return socketcall_cp(recvfrom, fd, buf, len, flags, addr, alen);
#endif
}
