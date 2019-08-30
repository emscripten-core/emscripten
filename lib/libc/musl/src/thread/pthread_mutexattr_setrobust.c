#include "pthread_impl.h"

int pthread_mutexattr_setrobust(pthread_mutexattr_t *a, int robust)
{
	if (robust > 1U) return EINVAL;
	a->__attr &= ~4;
	a->__attr |= robust*4;
	return 0;
}
