#include "pthread_impl.h"

int pthread_rwlockattr_setpshared(pthread_rwlockattr_t *a, int pshared)
{
	if (pshared > 1U) return EINVAL;
#ifdef __EMSCRIPTEN__
	if (pshared) return ENOTSUP;
#endif
	a->__attr[0] = pshared;
	return 0;
}
