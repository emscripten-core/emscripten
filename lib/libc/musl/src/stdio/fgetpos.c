#include "stdio_impl.h"

int fgetpos(FILE *restrict f, fpos_t *restrict pos)
{
	off_t off = __ftello(f);
	if (off < 0) return -1;
	*(off_t *)pos = off;
	return 0;
}

LFS64(fgetpos);
