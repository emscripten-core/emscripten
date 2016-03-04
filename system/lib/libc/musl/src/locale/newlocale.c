#include <stdlib.h>
#include <string.h>
#include "locale_impl.h"
#include "libc.h"

locale_t newlocale(int mask, const char *name, locale_t base)
{
	if (*name && strcmp(name, "C") && strcmp(name, "POSIX"))
		return 0;
	if (!base) {
		// XXX EMSCRIPTEN: avoid a malloc (which does time(), sysconf(), sbrk(), etc.) during startup
		static struct __locale_struct first;
		static int used_first = 0;
		if (!used_first) {
			used_first = 1;
			base = &first;
		} else {
			base = calloc(1, sizeof *base);
		}
	}
	return base;
}

weak_alias(newlocale, __newlocale);
