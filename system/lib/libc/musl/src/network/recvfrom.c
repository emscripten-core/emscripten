#include <sys/socket.h>
#include "syscall.h"
#ifdef __EMSCRIPTEN_PTHREADS__
#include "emscripten_fd_wait.h"
#endif

ssize_t recvfrom(int fd, void *restrict buf, size_t len, int flags, struct sockaddr *restrict addr, socklen_t *restrict alen)
{
#ifdef __EMSCRIPTEN_PTHREADS__
	return __syscall_ret(__emscripten_sock_retry_cp(fd, flags & MSG_DONTWAIT,
		__socketcall_cp(recvfrom, fd, buf, len, flags, addr, alen)));
#else
	return socketcall_cp(recvfrom, fd, buf, len, flags, addr, alen);
#endif
}
