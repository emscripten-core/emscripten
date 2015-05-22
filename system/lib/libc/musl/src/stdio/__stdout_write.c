#include "stdio_impl.h"
#include <termios.h>
#include <sys/ioctl.h>

size_t __stdout_write(FILE *f, const unsigned char *buf, size_t len)
{
	struct termios tio;
	f->write = __stdio_write;
	if (!(f->flags & F_SVB) && __syscall(SYS_ioctl, f->fd, TCGETS, &tio))
		f->lbf = -1;
	return __stdio_write(f, buf, len);
}
