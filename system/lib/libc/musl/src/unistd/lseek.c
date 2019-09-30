#include <unistd.h>
#include "syscall.h"
#include "libc.h"

off_t lseek(int fd, off_t offset, int whence)
{
#ifdef SYS__llseek
	off_t result;
#ifdef __EMSCRIPTEN__
	__wasi_errno_t err = __wasi_fd_seek(fd, offset, whence, &result);
  if (err != __WASI_ESUCCESS) {
		__wasi_syscall_ret(err);
		return -1;
	}
	return result;
#else
	return syscall(SYS__llseek, fd, offset>>32, offset, &result, whence) ? -1 : result;
#endif // __EMSCRIPTEN__
#else
	return syscall(SYS_lseek, fd, offset, whence);
#endif
}

LFS64(lseek);
