#include <string.h>

char *strcpy(char *restrict dest, const char *restrict src)
{
	const unsigned char *s = src;
	unsigned char *d = dest;
	while ((*d++ = *s++));
	return dest;
}
