#include "pthread_impl.h"
#include <errno.h>

int pthread_spin_lock(pthread_spinlock_t *s)
{
	while (*(_Atomic int *)s || a_cas((_Atomic int *)s, 0, EBUSY)) a_spin();
	return 0;
}
