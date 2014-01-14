#include "stdio_impl.h"

/* This function will never be called if there is already data
 * buffered for reading. Thus we can get by with very few branches. */

int __uflow(FILE *f)
{
	unsigned char c;
	if ((f->rend || !__toread(f)) && f->read(f, &c, 1)==1) return c;
	return EOF;
}
