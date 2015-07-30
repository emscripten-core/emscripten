/* 
 * This code was written by Rich Felker in 2010; no copyright is claimed.
 * This code is in the public domain. Attribution is appreciated but
 * unnecessary.
 */

#include <wchar.h>
#include <errno.h>
#include "internal.h"

int mbtowc(wchar_t *restrict wc, const char *restrict src, size_t n)
{
	unsigned c;
	const unsigned char *s = (const void *)src;

	if (!s) return 0;
	if (!n) goto ilseq;
	if (!wc) wc = (void *)&wc;

	if (*s < 0x80) return !!(*wc = *s);
	if (*s-SA > SB-SA) goto ilseq;
	c = bittab[*s++-SA];

	/* Avoid excessive checks against n: If shifting the state n-1
	 * times does not clear the high bit, then the value of n is
	 * insufficient to read a character */
	if (n<4 && ((c<<(6*n-6)) & (1U<<31))) goto ilseq;

	if (OOB(c,*s)) goto ilseq;
	c = c<<6 | *s++-0x80;
	if (!(c&(1U<<31))) {
		*wc = c;
		return 2;
	}

	if (*s-0x80u >= 0x40) goto ilseq;
	c = c<<6 | *s++-0x80;
	if (!(c&(1U<<31))) {
		*wc = c;
		return 3;
	}

	if (*s-0x80u >= 0x40) goto ilseq;
	*wc = c<<6 | *s++-0x80;
	return 4;

ilseq:
	errno = EILSEQ;
	return -1;
}
