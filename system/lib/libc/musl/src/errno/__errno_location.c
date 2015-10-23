#include "pthread_impl.h"

int *__errno_location(void)
{
	static int e;
	if (libc.main_thread) return __pthread_self()->errno_ptr;
	return &e;
}
