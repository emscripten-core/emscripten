#include "stdio_impl.h"

static unsigned char buf[BUFSIZ+UNGET];
static FILE f = {
	.buf = buf+UNGET,
	.buf_size = sizeof buf-UNGET,
	.fd = 1,
	.flags = F_PERM | F_NORD,
	.lbf = '\n',
	.write = __stdout_write,
	.seek = __stdio_seek,
	.close = __stdio_close,
	.lock = -1,
};
FILE *const stdout = &f;
FILE *volatile __stdout_used = &f;
