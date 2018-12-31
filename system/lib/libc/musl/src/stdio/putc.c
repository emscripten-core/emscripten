#include "stdio_impl.h"

int putc(int c, FILE *f)
{
	if (f->lock < 0 || !__lockfile(f))
		return putc_unlocked(c, f);
	c = putc_unlocked(c, f);
	__unlockfile(f);
	return c;
}

weak_alias(putc, _IO_putc);
