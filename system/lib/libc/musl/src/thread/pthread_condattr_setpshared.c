#include "pthread_impl.h"

int pthread_condattr_setpshared(pthread_condattr_t *a, int pshared)
{
	if (pshared > 1U) return EINVAL;
#ifdef __EMSCRIPTEN__
	if (pshared) return ENOTSUP;
#endif
	a->__attr &= 0x7fffffff;
	a->__attr |= (unsigned)pshared<<31;
	return 0;
}
