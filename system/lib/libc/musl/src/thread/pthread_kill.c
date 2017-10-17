#include "pthread_impl.h"

int pthread_kill(pthread_t t, int sig)
{
	int r;
	__lock(t->killlock);
	r = t->dead ? ESRCH : -__syscall(SYS_tkill, t->tid, sig);
	__unlock(t->killlock);
	return r;
}
