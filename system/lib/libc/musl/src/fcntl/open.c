#include <fcntl.h>
#include <stdarg.h>
#include "syscall.h"
#include "libc.h"

int open(const char *filename, int flags, ...)
{
	mode_t mode;
	va_list ap;
	va_start(ap, flags);
	mode = va_arg(ap, mode_t);
	va_end(ap);
	return syscall_cp(SYS_open, filename, flags|O_LARGEFILE, mode);
}

LFS64(open);
