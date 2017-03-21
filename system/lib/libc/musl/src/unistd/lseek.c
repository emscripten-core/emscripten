#include <unistd.h>
#include "syscall.h"
#include "libc.h"

off_t lseek(int fd, off_t offset, int whence)
{
#ifdef SYS__llseek
	off_t result;
	off_t offset_hi =
#ifndef __EMSCRIPTEN__
		offset >> 32;
#else
		0;
#endif
	return syscall(SYS__llseek, fd, offset_hi, offset, &result, whence) ? -1 : result;
#else
	return syscall(SYS_lseek, fd, offset, whence);
#endif
}

LFS64(lseek);
