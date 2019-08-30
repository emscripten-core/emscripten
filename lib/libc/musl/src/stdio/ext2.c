#include "stdio_impl.h"

size_t __freadahead(FILE *f)
{
	return f->rend - f->rpos;
}

const char *__freadptr(FILE *f, size_t *sizep)
{
	size_t size = f->rend - f->rpos;
	if (!size) return 0;
	*sizep = size;
	return (const char *)f->rpos;
}

void __freadptrinc(FILE *f, size_t inc)
{
	f->rpos += inc;
}

void __fseterr(FILE *f)
{
	f->flags |= F_ERR;
}
