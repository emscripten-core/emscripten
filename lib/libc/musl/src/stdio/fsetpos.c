#include "stdio_impl.h"

int fsetpos(FILE *f, const fpos_t *pos)
{
	return __fseeko(f, *(const off_t *)pos, SEEK_SET);
}

LFS64(fsetpos);
