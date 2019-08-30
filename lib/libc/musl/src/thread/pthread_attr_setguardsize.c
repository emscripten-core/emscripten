#include "pthread_impl.h"

int pthread_attr_setguardsize(pthread_attr_t *a, size_t size)
{
	if (size > SIZE_MAX/8) return EINVAL;
	a->_a_guardsize = size - DEFAULT_GUARD_SIZE;
	return 0;
}
