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

	FLOCK(f);

	if (!n || !s) {
		f->flags |= F_ERR;
		FUNLOCK(f);
		errno = EINVAL;
		return -1;
	}

	if (!*s) *n=0;

	for (;;) {
		z = memchr(f->rpos, delim, f->rend - f->rpos);
		k = z ? z - f->rpos + 1 : f->rend - f->rpos;
		if (i+k+1 >= *n) {
			if (k >= SIZE_MAX/2-i) goto oom;
			size_t m = i+k+2;
			if (!z && m < SIZE_MAX/4) m += m/2;
			tmp = realloc(*s, m);
			if (!tmp) {
				m = i+k+2;
				tmp = realloc(*s, m);
				if (!tmp) goto oom;
			}
			*s = tmp;
			*n = m;
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
	f->flags |= F_ERR;
	FUNLOCK(f);
	errno = ENOMEM;
	return -1;
}

weak_alias(getdelim, __getdelim);
