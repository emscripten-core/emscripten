#include <unistd.h>
#include "syscall.h"

int fsync(int fd)
{
#if __EMSCRIPTEN__
	return __wasi_syscall_ret(__wasi_fd_sync(fd));
#else
	return syscall_cp(SYS_fsync, fd);
#endif
}
