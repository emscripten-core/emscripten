#include "stdio_impl.h"
#include <wchar.h>
#include <limits.h>
#include <ctype.h>
#include <string.h> // XXX EMSCRIPTEN

wint_t __fputwc_unlocked(wchar_t c, FILE *f)
{
	char mbc[MB_LEN_MAX];
	int l;

#if 0 // XXX EMSCRIPTEN
	f->mode |= f->mode+1;

	if (isascii(c)) {
		c = putc_unlocked(c, f);
	} else if (f->wpos + MB_LEN_MAX < f->wend) {
		l = wctomb((void *)f->wpos, c);
		if (l < 0) c = WEOF;
		else f->wpos += l;
	} else {
		l = wctomb(mbc, c);
		if (l < 0 || __fwritex((void *)mbc, l, f) < l) c = WEOF;
	}
#else
    int i;

  memset(mbc, 0, sizeof(mbc));
	l = wctomb(mbc, c);
  if (l < 0) c = WEOF;
  else if (f != stdout && f != stderr) { // XXX
	  if (l < 4) l = 4;
  }
  for (int i = 0; i < l; i++) fputc(mbc[i], f);
#endif
	return c;
}

wint_t fputwc(wchar_t c, FILE *f)
{
	FLOCK(f);
	c = __fputwc_unlocked(c, f);
	FUNLOCK(f);
	return c;
}

weak_alias(__fputwc_unlocked, fputwc_unlocked);
weak_alias(__fputwc_unlocked, putwc_unlocked);
