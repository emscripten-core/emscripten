#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <ctype.h>
#include <wchar.h>
#include <wctype.h>
#include <limits.h>
#include <string.h>
#include <errno.h>
#include <math.h>
#include <float.h>

#include "stdio_impl.h"
#include "shgetc.h"
#include "intscan.h"
#include "floatscan.h"
#include "libc.h"

#define SIZE_hh -2
#define SIZE_h  -1
#define SIZE_def 0
#define SIZE_l   1
#define SIZE_L   2
#define SIZE_ll  3

static void store_int(void *dest, int size, unsigned long long i)
{
	if (!dest) return;
	switch (size) {
	case SIZE_hh:
		*(char *)dest = i;
		break;
	case SIZE_h:
		*(short *)dest = i;
		break;
	case SIZE_def:
		*(int *)dest = i;
		break;
	case SIZE_l:
		*(long *)dest = i;
		break;
	case SIZE_ll:
		*(long long *)dest = i;
		break;
	}
}

static void *arg_n(va_list ap, unsigned int n)
{
	void *p;
	unsigned int i;
	va_list ap2;
	va_copy(ap2, ap);
	for (i=n; i>1; i--) va_arg(ap2, void *);
	p = va_arg(ap2, void *);
	va_end(ap2);
	return p;
}

static int in_set(const wchar_t *set, int c)
{
	int j;
	const wchar_t *p = set;
	if (*p == '-') {
		if (c=='-') return 1;
		p++;
	} else if (*p == ']') {
		if (c==']') return 1;
		p++;
	}
	for (; *p && *p != ']'; p++) {
		if (*p=='-' && p[1] && p[1] != ']')
			for (j=p++[-1]; j<*p; j++)
				if (c==j) return 1;
		if (c==*p) return 1;
	}
	return 0;
}

#if 1
#undef getwc
#define getwc(f) \
	((f)->rpos < (f)->rend && *(f)->rpos < 128 ? *(f)->rpos++ : (getwc)(f))

#undef ungetwc
#define ungetwc(c,f) \
	((f)->rend && (c)<128U ? *--(f)->rpos : ungetwc((c),(f)))
#endif

int vfwscanf(FILE *restrict f, const wchar_t *restrict fmt, va_list ap)
{
	int width;
	int size;
	int alloc;
	const wchar_t *p;
	int c, t;
	char *s;
	wchar_t *wcs;
	void *dest=NULL;
	int invert;
	int matches=0;
	off_t pos = 0, cnt;
	static const char size_pfx[][3] = { "hh", "h", "", "l", "L", "ll" };
	char tmp[3*sizeof(int)+10];

	FLOCK(f);

	for (p=fmt; *p; p++) {

		if (iswspace(*p)) {
			while (iswspace(p[1])) p++;
			while (iswspace((c=getwc(f)))) pos++;
			ungetwc(c, f);
			continue;
		}
		if (*p != '%' || p[1] == '%') {
			p += *p=='%';
			c = getwc(f);
			if (c!=*p) {
				ungetwc(c, f);
				if (c<0) goto input_fail;
				goto match_fail;
			}
			pos++;
			continue;
		}

		p++;
		if (*p=='*') {
			dest = 0; p++;
		} else if (iswdigit(*p) && p[1]=='$') {
			dest = arg_n(ap, *p-'0'); p+=2;
		} else {
			dest = va_arg(ap, void *);
		}

		for (width=0; iswdigit(*p); p++) {
			width = 10*width + *p - '0';
		}

		if (*p=='m') {
			alloc = 1;
			p++;
		} else {
			alloc = 0;
		}

		size = SIZE_def;
		switch (*p++) {
		case 'h':
			if (*p == 'h') p++, size = SIZE_hh;
			else size = SIZE_h;
			break;
		case 'l':
			if (*p == 'l') p++, size = SIZE_ll;
			else size = SIZE_l;
			break;
		case 'j':
			size = SIZE_ll;
			break;
		case 'z':
		case 't':
			size = SIZE_l;
			break;
		case 'L':
			size = SIZE_L;
			break;
		case 'd': case 'i': case 'o': case 'u': case 'x':
		case 'a': case 'e': case 'f': case 'g':
		case 'A': case 'E': case 'F': case 'G': case 'X':
		case 's': case 'c': case '[':
		case 'S': case 'C':
		case 'p': case 'n':
			p--;
			break;
		default:
			goto fmt_fail;
		}

		t = *p;

		/* Transform ls,lc -> S,C */
		if (size==SIZE_l && (t&15)==3) t&=~32;

		if (t != 'n') {
			if (t != '[' && (t|32) != 'c')
				while (iswspace((c=getwc(f)))) pos++;
			else
				c=getwc(f);
			if (c < 0) goto input_fail;
			ungetwc(c, f);
		}

		switch (t) {
		case 'n':
			store_int(dest, size, pos);
			/* do not increment match count, etc! */
			continue;

		case 'c':
			if (width < 1) width = 1;
			s = dest;
			for (; width && (c=getwc(f)) >= 0; width--) {
				int l = wctomb(s?s:tmp, c);
				if (l<0) goto input_fail;
				if (s) s+=l;
				pos++;
			}
			if (width) goto match_fail;
			break;

		case 'C':
			if (width < 1) width = 1;
			wcs = dest;
			for (; width && (c=getwc(f)) >= 0; width--)
				pos++, wcs && (*wcs++ = c);
			if (width) goto match_fail;
			break;

		case 's':
			if (width < 1) width = -1;
			s = dest;
			while (width && !iswspace(c=getwc(f)) && c!=EOF) {
				int l = wctomb(s?s:tmp, c);
				if (l<0) goto input_fail;
				if (s) s+=l;
				pos++;
				width-=(width>0);
			}
			if (width) ungetwc(c, f);
			if (s) *s = 0;
			break;

		case 'S':
			wcs = dest;
			if (width < 1) width = -1;
			while (width && !iswspace(c=getwc(f)) && c!=EOF)
				width-=(width>0), pos++, *wcs++ = c;
			if (width) ungetwc(c, f);
			if (wcs) *wcs = 0;
			break;

		case '[':
			s = (size == SIZE_def) ? dest : 0;
			wcs = (size == SIZE_l) ? dest : 0;

			if (*++p == '^') p++, invert = 1;
			else invert = 0;

			int gotmatch = 0;

			if (width < 1) width = -1;

			while (width) {
				if ((c=getwc(f))<0) break;
				if (in_set(p, c) == invert)
					break;
				if (wcs) {
					*wcs++ = c;
				} else if (size != SIZE_l) {
					int l = wctomb(s?s:tmp, c);
					if (l<0) goto input_fail;
					if (s) s+=l;
				}
				pos++;
				width-=(width>0);
				gotmatch=1;
			}
			if (width) ungetwc(c, f);

			if (!gotmatch) goto match_fail;

			if (*p==']') p++;
			while (*p!=']') {
				if (!*p) goto fmt_fail;
				p++;
			}

			if (wcs) *wcs++ = 0;
			if (s) *s++ = 0;
			break;

		case 'd': case 'i': case 'o': case 'u': case 'x':
		case 'a': case 'e': case 'f': case 'g':
		case 'A': case 'E': case 'F': case 'G': case 'X':
		case 'p':
			if (width < 1) width = 0;
			snprintf(tmp, sizeof tmp, "%.*s%.0d%s%c%%lln",
				1+!dest, "%*", width, size_pfx[size+2], t);
			cnt = 0;
			if (fscanf(f, tmp, dest?dest:&cnt, &cnt) == -1)
				goto input_fail;
			else if (!cnt)
				goto match_fail;
			pos += cnt;
			break;
		default:
			goto fmt_fail;
		}

		if (dest) matches++;
	}
	if (0) {
fmt_fail:
input_fail:
		if (!matches) matches--;
	}
match_fail:
	FUNLOCK(f);
	return matches;
}

weak_alias(vfwscanf,__isoc99_vfwscanf);