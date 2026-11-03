#include <netdb.h>
#include "pthread_impl.h"

#undef h_errno
int h_errno;

#ifdef __EMSCRIPTEN__
static _Thread_local int __h_errno_storage;
#endif

int *__h_errno_location(void)
{
#ifdef __EMSCRIPTEN__
	return &__h_errno_storage;
#else
	if (!__pthread_self()->stack) return &h_errno;
	return &__pthread_self()->h_errno_val;
#endif
}
