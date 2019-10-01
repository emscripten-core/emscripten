#include "stdio_impl.h"

off_t __stdio_seek(FILE *f, off_t off, int whence)
{
	off_t ret;
#ifdef SYS__llseek
#ifdef __EMSCRIPTEN__
	if (__wasi_syscall_ret(__wasi_fd_seek(f->fd, off, whence, &ret)))
#else
	if (syscall(SYS__llseek, f->fd, off>>32, off, &ret, whence)<0)
#endif // EMSCRIPTEN
		ret = -1;
#else
	ret = syscall(SYS_lseek, f->fd, off, whence);
#endif
	return ret;
}
