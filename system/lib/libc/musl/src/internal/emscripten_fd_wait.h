#ifndef EMSCRIPTEN_FD_WAIT_H
#define EMSCRIPTEN_FD_WAIT_H

// Blocking socket data ops on emscripten: the underlying JS syscalls are
// strictly synchronous and return -EAGAIN when they would block. For a
// blocking fd the network wrappers wait for readiness via the single blocking
// primitive _emscripten_fd_wait and retry. This is a pthreads-only facility
// (the retry loops compile only into the -mt libc): _emscripten_fd_wait blocks
// by parking a proxied worker on its sync-proxy. Where no stack can wait (the
// event-loop thread itself), the wait fails and the EAGAIN surfaces unchanged.
// Single-threaded JSPI/ASYNCIFY builds use epoll for readiness instead.

#include <fcntl.h>
#include <errno.h>
#include <poll.h>
#include "syscall.h"

int _emscripten_fd_wait(int fd, int events);

static inline int __emscripten_sock_can_wait(int fd, int dontwait)
{
	if (dontwait) return 0;
	int fl = __syscall(SYS_fcntl64, fd, F_GETFL);
	return fl >= 0 && !(fl & O_NONBLOCK);
}

// The blocking-socket retry convention: `attempt` is a strictly synchronous
// __socketcall_cp expression returning -EAGAIN when it would block. On EAGAIN
// with a blocking fd (and no MSG_DONTWAIT), wait for readiness and retry. If
// the wait itself fails (no thread to park on), the EAGAIN surfaces unchanged.
// Yields the raw syscall result; callers apply __syscall_ret.
#define __emscripten_sock_retry_cp(fd, dontwait, attempt) ({ \
	long __r; \
	for (;;) { \
		__r = (attempt); \
		if (__r != -EAGAIN || !__emscripten_sock_can_wait(fd, dontwait) \
		    || _emscripten_fd_wait(fd, POLLIN)) break; \
	} \
	__r; \
})

#endif
