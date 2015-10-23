#include "stdio_impl.h"
#include <wchar.h>
#include <errno.h>

wint_t __fgetwc_unlocked(FILE *f)
{
	mbstate_t st = { 0 };
	wchar_t wc;
	int c;
	unsigned char b;
	size_t l;

	f->mode |= f->mode+1;

	/* Convert character from buffer if possible */
	if (f->rpos < f->rend) {
		l = mbrtowc(&wc, (void *)f->rpos, f->rend - f->rpos, &st);
		if (l+2 >= 2) {
			f->rpos += l + !l; /* l==0 means 1 byte, null */
			return wc;
		}
		if (l == -1) {
			f->rpos++;
			return WEOF;
		}
	} else l = -2;

	/* Convert character byte-by-byte */
	while (l == -2) {
		b = c = getc_unlocked(f);
		if (c < 0) {
			if (!mbsinit(&st)) errno = EILSEQ;
			return WEOF;
		}
		l = mbrtowc(&wc, (void *)&b, 1, &st);
		if (l == -1) return WEOF;
	}

	return wc;
}

wint_t fgetwc(FILE *f)
{
	wint_t c;
	FLOCK(f);
	c = __fgetwc_unlocked(f);
	FUNLOCK(f);
	return c;
}

weak_alias(__fgetwc_unlocked, fgetwc_unlocked);
weak_alias(__fgetwc_unlocked, getwc_unlocked);
