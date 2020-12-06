#include "stdio_impl.h"
#ifndef __EMSCRIPTEN__
#include <unistd.h>
#endif

off_t __stdio_seek(FILE *f, off_t off, int whence)
{
#ifdef __EMSCRIPTEN__
	off_t ret;
	if (__wasi_syscall_ret(__wasi_fd_seek(f->fd, off, whence, &ret)))
		ret = -1;
	return ret;
#else
	return __lseek(f->fd, off, whence);
#endif // __EMSCRIPTEN__
}
