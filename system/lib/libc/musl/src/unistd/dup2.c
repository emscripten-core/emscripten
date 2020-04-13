#ifdef __EMSCRIPTEN__
#include <stropts.h>
#endif
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include "syscall.h"

int dup2(int old, int new)
{
	int r;
#ifdef SYS_dup2
	while ((r=__syscall(SYS_dup2, old, new))==-EBUSY);
#else
	if (old==new) {
#if __EMSCRIPTEN__
		r = __syscall(SYS_fcntl, old, F_GETFD);
#else
		r = __wasi_fd_is_valid(old);
#endif
		if (r >= 0) return old;
	} else {
		while ((r=__syscall(SYS_dup3, old, new, 0))==-EBUSY);
	}
#endif
	return __syscall_ret(r);
}
