#include "stdio_impl.h"

int ungetc(int c, FILE *f)
{
	if (c == EOF) return c;

	FLOCK(f);

	if ((!f->rend && __toread(f)) || f->rpos <= f->buf - UNGET) {
		FUNLOCK(f);
		return EOF;
	}

	*--f->rpos = c;
	f->flags &= ~F_EOF;

	FUNLOCK(f);
	return c;
}
