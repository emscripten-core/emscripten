#include "stdio_impl.h"
#include <string.h>
#include <inttypes.h>
#include <errno.h>

#define MIN(a,b) ((a)<(b) ? (a) : (b))

ssize_t getdelim(char **restrict s, size_t *restrict n, int delim, FILE *restrict f)
{
	char *tmp;
	unsigned char *z;
	size_t k;
	size_t i=0;
	int c;

	if (!n || !s) {
		errno = EINVAL;
		return -1;
	}

	if (!*s) *n=0;

	FLOCK(f);

	for (;;) {
		z = memchr(f->rpos, delim, f->rend - f->rpos);
		k = z ? z - f->rpos + 1 : f->rend - f->rpos;
		if (i+k >= *n) {
			if (k >= SIZE_MAX/2-i) goto oom;
			*n = i+k+2;
			if (*n < SIZE_MAX/4) *n *= 2;
			tmp = realloc(*s, *n);
			if (!tmp) {
				*n = i+k+2;
				tmp = realloc(*s, *n);
				if (!tmp) goto oom;
			}
			*s = tmp;
		}
		memcpy(*s+i, f->rpos, k);
		f->rpos += k;
		i += k;
		if (z) break;
		if ((c = getc_unlocked(f)) == EOF) {
			if (!i || !feof(f)) {
				FUNLOCK(f);
				return -1;
			}
			break;
		}
		if (((*s)[i++] = c) == delim) break;
	}
	(*s)[i] = 0;

	FUNLOCK(f);

	return i;
oom:
	FUNLOCK(f);
	errno = ENOMEM;
	return -1;
}

weak_alias(getdelim, __getdelim);
