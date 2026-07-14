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

#endif
