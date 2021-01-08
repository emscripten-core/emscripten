#include "pthread_impl.h"

int pthread_setschedprio(pthread_t t, int prio)
{
#ifdef __EMSCRIPTEN__ // XXX Emscripten web or Node workers doesn't support prioritizing threads
	// no-op
	return 0;
#else
	int r;
	__lock(t->killlock);
	r = t->dead ? ESRCH : -__syscall(SYS_sched_setparam, t->tid, &prio);
	__unlock(t->killlock);
	return r;
#endif
}
