#include "pthread_impl.h"

int pthread_detach(pthread_t t)
{
	/* Cannot detach a thread that's already exiting */
	if (a_swap(t->exitlock, 1))
		return pthread_join(t, 0);
	t->detached = 2;
	__unlock(t->exitlock);
	return 0;
}
