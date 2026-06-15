#include "pthread_impl.h"

int pthread_barrierattr_setpshared(pthread_barrierattr_t *a, int pshared)
{
	if (pshared > 1U) return EINVAL;
#ifdef __EMSCRIPTEN__
	if (pshared) return ENOTSUP;
#endif
	a->__attr = pshared ? INT_MIN : 0;
	return 0;
}
