#include "pthread_impl.h"
#include <errno.h>

int pthread_spin_trylock(pthread_spinlock_t *s)
{
	return a_cas((_Atomic int *)s, 0, EBUSY);
}
