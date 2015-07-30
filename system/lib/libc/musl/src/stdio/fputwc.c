#include "stdio_impl.h"
#include <wchar.h>
#include <limits.h>
#include <ctype.h>

wint_t __fputwc_unlocked(wchar_t c, FILE *f)
{
	char mbc[MB_LEN_MAX];
	int l;

#ifndef __EMSCRIPTEN__
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
	return c;
#else
  if (isascii(c)) {
    c = fputc(c, f);
  } else {
    l = wctomb(mbc, c);
    if (l < 0) c = WEOF;
    else {
      for (int i = 0; i < l; i++) {
        if (fputc(mbc[i], f) == EOF) {
          c = WEOF;
          break;
        }
      }
    }
  }
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
