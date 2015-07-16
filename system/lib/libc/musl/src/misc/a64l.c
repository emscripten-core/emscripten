#include <stdlib.h>
#include <string.h>
#include <stdint.h>

static const char digits[] =
	"./0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";

long a64l(const char *s)
{
	int e;
	uint32_t x = 0;
	for (e=0; e<36 && *s; e+=6, s++)
		x |= (strchr(digits, *s)-digits)<<e;
	return x;
}

char *l64a(long x0)
{
	static char s[7];
	char *p;
	uint32_t x = x0;
	for (p=s; x; p++, x>>=6)
		*p = digits[x&63];
	*p = 0;
	return s;
}
