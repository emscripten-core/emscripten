#include <stdlib.h>
#include <string.h>
#include "locale_impl.h"
#include "libc.h"

locale_t newlocale(int mask, const char *name, locale_t base)
{
	if (*name && strcmp(name, "C") && strcmp(name, "POSIX"))
		return 0;
	if (!base) base = calloc(1, sizeof *base);
	return base;
}

weak_alias(newlocale, __newlocale);
