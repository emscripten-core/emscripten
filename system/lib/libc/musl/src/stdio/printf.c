#include "stdio_impl.h"
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

// XXX EMSCRIPTEN
int iprintf(const char *restrict fmt, ...)
{
	int ret;
	va_list ap;
	va_start(ap, fmt);
	ret = vfiprintf(stdout, fmt, ap);
	va_end(ap);
	return ret;
}

int __small_printf(const char *restrict fmt, ...)
{
	int ret;
	va_list ap;
	va_start(ap, fmt);
	ret = __small_vfprintf(stdout, fmt, ap);
	va_end(ap);
	return ret;
}

