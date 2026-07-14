#include <sys/socket.h>
#include "syscall.h"
#ifdef __EMSCRIPTEN_PTHREADS__
#include "emscripten_fd_wait.h"
#endif

int accept(int fd, struct sockaddr *restrict addr, socklen_t *restrict len)
{
#ifdef __EMSCRIPTEN_PTHREADS__
	for (;;) {
		long r = __socketcall_cp(accept, fd, addr, len, 0, 0, 0);
		if (r != -EAGAIN || !__emscripten_sock_can_wait(fd, 0)
		    || _emscripten_fd_wait(fd, POLLIN))
			return __syscall_ret(r);
	}
#else
	return socketcall_cp(accept, fd, addr, len, 0, 0, 0);
#endif
}
