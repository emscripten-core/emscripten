#define _GNU_SOURCE
#include <fcntl.h>
#include <stdarg.h>
#include <errno.h>
#include "syscall.h"

#ifdef __EMSCRIPTEN__
__attribute__((no_sanitize("address")))
#endif
int fcntl(int fd, int cmd, ...)
{
	unsigned long arg;
	// XXX Emscripten: According to the va_arg man page it is undefined behaviour to
	// read arguments that are not passed.  This can lead to a false positive
	// in SAFE_HEAP, so avoid it.
	if (cmd != F_GETFL && cmd != F_GETFD && cmd != F_GETOWN) {
		va_list ap;
		va_start(ap, cmd);
		arg = va_arg(ap, unsigned long);
		va_end(ap);
	}
	if (cmd == F_SETFL) arg |= O_LARGEFILE;
	if (cmd == F_SETLKW) return syscall_cp(SYS_fcntl, fd, cmd, (void *)arg);
	if (cmd == F_GETOWN) {
		struct f_owner_ex ex;
		int ret = __syscall(SYS_fcntl, fd, F_GETOWN_EX, &ex);
#ifdef __EMSCRIPTEN__
		// XXX Emscripten: Mirror the behaviour/limitation of glibc which
		// will misinterpret negative PIDs in range of -1 to -4095 as being
		// errno values.
		if (ret == -EINVAL) ret = __syscall(SYS_fcntl, fd, cmd, (void *)arg);
#else
		if (ret == -EINVAL) return __syscall(SYS_fcntl, fd, cmd, (void *)arg);
#endif
		if (ret) return __syscall_ret(ret);
		return ex.type == F_OWNER_PGRP ? -ex.pid : ex.pid;
	}
	if (cmd == F_DUPFD_CLOEXEC) {
		int ret = __syscall(SYS_fcntl, fd, F_DUPFD_CLOEXEC, arg);
		if (ret != -EINVAL) {
#ifndef __EMSCRIPTEN__ // CLOEXEC makes no sense for a single process
			if (ret >= 0) __syscall(SYS_fcntl, ret, F_SETFD, FD_CLOEXEC);
#endif
			return __syscall_ret(ret);
		}
		ret = __syscall(SYS_fcntl, fd, F_DUPFD_CLOEXEC, 0);
		if (ret != -EINVAL) {
#ifdef __EMSCRIPTEN__
			if (ret >= 0) __wasi_fd_close(ret);
#else
			if (ret >= 0) __syscall(SYS_close, ret);
#endif
			return __syscall_ret(-EINVAL);
		}
		ret = __syscall(SYS_fcntl, fd, F_DUPFD, arg);
#ifndef __EMSCRIPTEN__ // CLOEXEC makes no sense for a single process
		if (ret >= 0) __syscall(SYS_fcntl, ret, F_SETFD, FD_CLOEXEC);
#endif
		return __syscall_ret(ret);
	}
	switch (cmd) {
	case F_SETLK:
	case F_GETLK:
	case F_GETOWN_EX:
	case F_SETOWN_EX:
		return syscall(SYS_fcntl, fd, cmd, (void *)arg);
	default:
		return syscall(SYS_fcntl, fd, cmd, arg);
	}
}
