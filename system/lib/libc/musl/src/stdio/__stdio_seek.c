#include "stdio_impl.h"

off_t __stdio_seek(FILE *f, off_t off, int whence)
{
	off_t ret;
#ifdef SYS__llseek
	off_t off_hi =
#ifndef __EMSCRIPTEN__
		off >> 32;
#else
		0;
#endif
	if (syscall(SYS__llseek, f->fd, off_hi, off, &ret, whence)<0)
		ret = -1;
#else
	ret = syscall(SYS_lseek, f->fd, off, whence);
#endif
	return ret;
}
