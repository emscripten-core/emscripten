#include <semaphore.h>
#include "pthread_impl.h"

static void cleanup(void *p)
{
	a_dec(p);
}

int sem_timedwait(sem_t *restrict sem, const struct timespec *restrict at)
{
	while (sem_trywait(sem)) {
		int r;
		a_inc(sem->__val+1);
		a_cas(sem->__val, 0, -1);
		r = __timedwait(sem->__val, -1, CLOCK_REALTIME, at, cleanup, sem->__val+1, 0);
		a_dec(sem->__val+1);
		if (r) {
			errno = r;
			return -1;
		}
	}
	return 0;
}
