#include <unistd.h>
#include "syscall.h"

off_t __lseek(int fd, off_t offset, int whence)
{
#ifdef __EMSCRIPTEN__
	off_t result;
	return __wasi_syscall_ret(__wasi_fd_seek(fd, offset, whence, &result)) ? -1 : result;
#else
#ifdef SYS__llseek
	off_t result;
	return syscall(SYS__llseek, fd, offset>>32, offset, &result, whence) ? -1 : result;
#else
	return syscall(SYS_lseek, fd, offset, whence);
#endif
#endif // __EMSCRIPTEN__
}

weak_alias(__lseek, lseek);
weak_alias(__lseek, lseek64);
