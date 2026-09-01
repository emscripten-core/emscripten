#define _GNU_SOURCE
#include <string.h>

char *strcasestr(const char *h, const char *n)
{
	size_t l = strlen(n);
	if (!l) return (char *)h;
	for (; *h; h++) if (!strncasecmp(h, n, l)) return (char *)h;
	return 0;
}
