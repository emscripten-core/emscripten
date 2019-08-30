#include "stdio_impl.h"

static unsigned char buf[UNGET];
static FILE f = {
	.buf = buf+UNGET,
	.buf_size = 0,
	.fd = 2,
	.flags = F_PERM | F_NORD,
	.lbf = -1,
	.write = __stdio_write,
	.seek = __stdio_seek,
	.close = __stdio_close,
	.lock = -1,
};
FILE *const stderr = &f;
FILE *volatile __stderr_used = &f;
