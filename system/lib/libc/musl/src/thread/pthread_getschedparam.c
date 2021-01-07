#include "pthread_impl.h"

int pthread_getschedparam(pthread_t t, int *restrict policy, struct sched_param *restrict param)
{
#ifdef __EMSCRIPTEN__ // XXX Emscripten web or Node workers doesn't support prioritizing threads
	// no-op
	return 0;
#else
	int r;
	__lock(t->killlock);
	if (t->dead) {
		r = ESRCH;
	} else {
		r = -__syscall(SYS_sched_getparam, t->tid, param);
		if (!r) {
			*policy = __syscall(SYS_sched_getscheduler, t->tid);
		}
	}
	__unlock(t->killlock);
	return r;
#endif
}
