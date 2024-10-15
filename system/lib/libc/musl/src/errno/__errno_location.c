#include <errno.h>
#include "pthread_impl.h"

#if __EMSCRIPTEN__
// For emscripten we use TLS here instead of `__pthread_self`, so that in single
// threaded builds this gets lowered away to normal global variable.
static _Thread_local int __errno_storage = 0;
#endif

int *__errno_location(void)
{
#if __EMSCRIPTEN__
	return &__errno_storage;
#else
	return &__pthread_self()->errno_val;
#endif
}

weak_alias(__errno_location, ___errno_location);
