#include "stdio_impl.h"

int getc(FILE *f)
{
	int c;
	if (f->lock < 0 || !__lockfile(f))
		return getc_unlocked(f);
	c = getc_unlocked(f);
	__unlockfile(f);
	return c;
}

weak_alias(getc, _IO_getc);
