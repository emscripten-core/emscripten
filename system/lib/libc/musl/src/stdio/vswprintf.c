#include "stdio_impl.h"
#include <limits.h>
#include <string.h>
#include <errno.h>
#include <stdint.h>
#include <wchar.h>

struct cookie {
	wchar_t *ws;
	size_t l;
};

static size_t sw_write(FILE *f, const unsigned char *s, size_t l)
{
	size_t l0 = l;
	int i = 0;
	struct cookie *c = f->cookie;
	if (s!=f->wbase && sw_write(f, f->wbase, f->wpos-f->wbase)==-1)
		return -1;
	while (c->l && l && (i=mbtowc(c->ws, (void *)s, l))>=0) {
		s+=i;
		l-=i;
		c->l--;
		c->ws++;
	}
	*c->ws = 0;
	return i<0 ? i : l0;
}

int vswprintf(wchar_t *restrict s, size_t n, const wchar_t *restrict fmt, va_list ap)
{
#if 0 // XXX EMSCRIPTEN
	int r;
	FILE f;
	unsigned char buf[256];
	struct cookie c = { s, n-1 };

	memset(&f, 0, sizeof(FILE));
	f.lbf = EOF;
	f.write = sw_write;
	f.buf_size = sizeof buf;
	f.buf = buf;
	f.lock = -1;
	f.cookie = &c;
	if (!n) {
		return -1;
	} else if (n > INT_MAX) {
		errno = EOVERFLOW;
		return -1;
	}
	r = vfwprintf(&f, fmt, ap);
	sw_write(&f, 0, 0);
	return r>=n ? -1 : r;
#else
  // XXX EMSCRIPTEN: use memfs through libc fs
  #define TEMPFILE "emscripten.vswprintf.temp.buffer"
  FILE *f = fopen(TEMPFILE, "wb");
  int r = vfwprintf(f, fmt, ap);
  fclose(f);
  f = fopen(TEMPFILE, "rb");
  fread(s, 1, (r>=n ? n : r)*4, f);
  fclose(f);
  remove(TEMPFILE);
  return r>=n ? -1 : r;
#endif
}
