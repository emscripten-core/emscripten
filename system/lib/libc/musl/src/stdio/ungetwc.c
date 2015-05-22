#include "stdio_impl.h"
#include <wchar.h>
#include <limits.h>
#include <ctype.h>
#include <string.h>

wint_t ungetwc(wint_t c, FILE *f)
{
	unsigned char mbc[MB_LEN_MAX];
	int l=1;

	if (c == WEOF) return c;

	/* Try conversion early so we can fail without locking if invalid */
	if (!isascii(c) && (l = wctomb((void *)mbc, c)) < 0)
		return WEOF;

	FLOCK(f);

	f->mode |= f->mode+1;

	if ((!f->rend && __toread(f)) || f->rpos < f->buf - UNGET + l) {
		FUNLOCK(f);
		return EOF;
	}

	if (isascii(c)) *--f->rpos = c;
	else memcpy(f->rpos -= l, mbc, l);

	f->flags &= ~F_EOF;

	FUNLOCK(f);
	return c;
}
