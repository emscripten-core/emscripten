#include "pthread_impl.h"

int pthread_setschedprio(pthread_t t, int prio)
{
	int r;
	__lock(t->killlock);
	r = t->dead ? ESRCH : -__syscall(SYS_sched_setparam, t->tid, &prio);
	__unlock(t->killlock);
	return r;
}
