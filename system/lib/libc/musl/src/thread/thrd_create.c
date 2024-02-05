#include "pthread_impl.h"
#include <threads.h>

#ifdef __EMSCRIPTEN__
// Fix for lsan.  Since lsan wraps calls to the public `pthread_create` function
// if we call the internal __pthread_create function here to don't the wrapping
// See pthread_create wrapper in compiler-rt/lib/lsan/lsan_interceptors.cpp.
#define __pthread_create pthread_create
#else
int __pthread_create(pthread_t *restrict, const pthread_attr_t *restrict, void *(*)(void *), void *restrict);
#endif

int thrd_create(thrd_t *thr, thrd_start_t func, void *arg)
{
	int ret = __pthread_create(thr, __ATTRP_C11_THREAD, (void *(*)(void *))func, arg);
	switch (ret) {
	case 0:      return thrd_success;
	case EAGAIN: return thrd_nomem;
	default:     return thrd_error;
	}
}
