#include <string.h>

void *memchr(const void *s, int c, size_t n) {
	const char *t = s;
	for (size_t i = 0; i < n; ++i, ++t)
		if (*t == c)
			return (void *) t;
	return NULL;
}
