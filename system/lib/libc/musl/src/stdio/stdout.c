#include "stdio_impl.h"

#if __EMSCRIPTEN__
// Emscripten doesn't support terminal seeking.
static off_t __emscripten_stdout_seek(FILE *f, off_t off, int whence)
{
	return 0;
}

// No special work is needed to close stdout.
static int __emscripten_stdout_close(FILE *f)
{
	return 0;
}
#endif

#undef stdout

static unsigned char buf[BUFSIZ+UNGET];
hidden FILE __stdout_FILE = {
	.buf = buf+UNGET,
	.buf_size = sizeof buf-UNGET,
	.fd = 1,
	.flags = F_PERM | F_NORD,
	.lbf = '\n',
#if __EMSCRIPTEN__
	// avoid stout_write which adds special terminal window size handling, which emscripten doesn't support anyhow
	.write = __stdio_write,
	.seek = __emscripten_stdout_seek,
	.close = __emscripten_stdout_close,
#else
	.write = __stdout_write,
	.seek = __stdio_seek,
	.close = __stdio_close,
#endif
	.lock = -1,
};
FILE *const stdout = &__stdout_FILE;
FILE *volatile __stdout_used = &__stdout_FILE;
