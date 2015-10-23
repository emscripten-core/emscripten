/* 
 * This code was written by Rich Felker in 2010; no copyright is claimed.
 * This code is in the public domain. Attribution is appreciated but
 * unnecessary.
 */

#include <stdlib.h>

int mblen(const char *s, size_t n)
{
	return mbtowc(0, s, n);
}
