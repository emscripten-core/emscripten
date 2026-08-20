#include <sys/socket.h>
#include "syscall.h"
#ifdef __EMSCRIPTEN_PTHREADS__
#include "emscripten_fd_wait.h"
#endif

int accept(int fd, struct sockaddr *restrict addr, socklen_t *restrict len)
{
#ifdef __EMSCRIPTEN_PTHREADS__
	return __syscall_ret(__emscripten_sock_retry_cp(fd, 0,
		__socketcall_cp(accept, fd, addr, len, 0, 0, 0)));
#else
	return socketcall_cp(accept, fd, addr, len, 0, 0, 0);
#endif
}
