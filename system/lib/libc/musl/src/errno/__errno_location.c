#include "pthread_impl.h"

#if __EMSCRIPTEN_PTHREADS__
// for pthreads, use the proper location on the thread info, so each
// thread has its own errno
int *__errno_location(void)
{
	return &__pthread_self()->errno_val;
}
#else
// for single-threaded mode, avoid linking in pthreads support code
// just for this
static int __errno_storage = 0;

int *__errno_location(void)
{
	return &__errno_storage;
}
#endif
