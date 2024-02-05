#include "pthread_impl.h"
#include "lock.h"

int pthread_setschedprio(pthread_t t, int prio)
{
#ifdef __EMSCRIPTEN__ // XXX Emscripten web or Node workers doesn't support prioritizing threads
	// no-op
	return 0;
#else
	int r;
	sigset_t set;
	__block_app_sigs(&set);
	LOCK(t->killlock);
	r = !t->tid ? ESRCH : -__syscall(SYS_sched_setparam, t->tid, &prio);
	UNLOCK(t->killlock);
	__restore_sigs(&set);
	return r;
#endif
}
