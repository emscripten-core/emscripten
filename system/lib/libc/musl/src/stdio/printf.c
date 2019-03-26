#include <stdio.h>
#include <stdarg.h>

int printf(const char *restrict fmt, ...)
{
	int ret;
	va_list ap;
	va_start(ap, fmt);
	ret = vfprintf(stdout, fmt, ap);
	va_end(ap);
	return ret;
}

extern int __vfprintf_internal(FILE *restrict f, const char *restrict fmt, va_list ap, fmt_fp_t fmt_fp);

// XXX EMSCRIPTEN
int iprintf(const char *restrict fmt, ...)
{
	int ret;
	va_list ap;
	va_start(ap, fmt);
	ret = __vfprintf_internal(stdout, fmt, ap, NULL);
	va_end(ap);
	return ret;
}
