#include "pthread_impl.h"

int pthread_spin_trylock(pthread_spinlock_t *s)
{
	return -a_swap(s, 1) & EBUSY;
}
