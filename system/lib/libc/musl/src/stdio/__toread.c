#include <stdio_impl.h>

int __toread(FILE *f)
{
	f->mode |= f->mode-1;
	if (f->wpos > f->buf) f->write(f, 0, 0);
	f->wpos = f->wbase = f->wend = 0;
	if (f->flags & (F_EOF|F_NORD)) {
		if (f->flags & F_NORD) f->flags |= F_ERR;
		return EOF;
	}
	f->rpos = f->rend = f->buf;
	return 0;
}

#ifndef __EMSCRIPTEN__
static const int dummy = 0;
weak_alias(dummy, __towrite_used);

void __stdio_exit(void);

void __seek_on_exit()
{
	if (!__towrite_used) __stdio_exit();
}
#endif
