#define _GNU_SOURCE
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>

#define GUESS 240U

int vasprintf(char **s, const char *fmt, va_list ap)
{
	va_list ap2;
	char *a;
	int l=GUESS;

	if (!(a=malloc(GUESS))) return -1;

	va_copy(ap2, ap);
	l=vsnprintf(a, GUESS, fmt, ap2);
	va_end(ap2);

	if (l<GUESS) {
		char *b = realloc(a, l+1U);
		*s = b ? b : a;
		return l;
	}
	free(a);
	if (l<0 || !(*s=malloc(l+1U))) return -1;
	return vsnprintf(*s, l+1U, fmt, ap);
}
