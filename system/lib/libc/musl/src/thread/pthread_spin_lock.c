#include "pthread_impl.h"

int pthread_spin_lock(pthread_spinlock_t *s)
{
	while (a_swap(s, 1)) a_spin();
	return 0;
}
