#include <string.h>

char *__stpncpy(char *, const char *, size_t);

char *strncpy(char *restrict d, const char *restrict s, size_t n)
{
	__stpncpy(d, s, n);
	return d;
}
