#include "pthread_impl.h"

int pthread_spin_unlock(pthread_spinlock_t *s)
{
	a_store((_Atomic int *)s, 0);
	return 0;
}
