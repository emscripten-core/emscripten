#include "stdio_impl.h"
#include <stdio.h>
#include <limits.h>

int vsprintf(char *restrict s, const char *restrict fmt, va_list ap)
{
	return vsnprintf(s, INT_MAX, fmt, ap);
}

// XXX EMSCRIPTEN
int vsiprintf(char *restrict s, const char *restrict fmt, va_list ap)
{
	return vsniprintf(s, INT_MAX, fmt, ap);
}

int __small_vsprintf(char *restrict s, const char *restrict fmt, va_list ap)
{
	return __small_vsnprintf(s, INT_MAX, fmt, ap);
}

