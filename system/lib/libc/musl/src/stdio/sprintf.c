#include "stdio_impl.h"
#include <stdio.h>
#include <stdarg.h>

int sprintf(char *restrict s, const char *restrict fmt, ...)
{
	int ret;
	va_list ap;
	va_start(ap, fmt);
	ret = vsprintf(s, fmt, ap);
	va_end(ap);
	return ret;
}

// XXX EMSCRIPTEN
int siprintf(char *restrict s, const char *restrict fmt, ...)
{
	int ret;
	va_list ap;
	va_start(ap, fmt);
	ret = vsiprintf(s, fmt, ap);
	va_end(ap);
	return ret;
}

int __small_sprintf(char *restrict s, const char *restrict fmt, ...)
{
	int ret;
	va_list ap;
	va_start(ap, fmt);
	ret = __small_vsprintf(s, fmt, ap);
	va_end(ap);
	return ret;
}

