#define _GNU_SOURCE
#include <unistd.h>
#include "libc.h"

int getpagesize(void)
{
#if __EMSCRIPTEN__
	// A value used historically in emscripten, and which so far we don't have a
	// strong reason to change.
	return 16384;
#else
	return PAGE_SIZE;
#endif
}
