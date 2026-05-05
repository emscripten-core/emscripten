#include "pthread_impl.h"
#include "lock.h"

int pthread_getschedparam(pthread_t t, int *restrict policy, struct sched_param *restrict param)
{
#ifdef __EMSCRIPTEN__ // XXX Emscripten web or Node workers doesn't support prioritizing threads
	// no-op
	return 0;
#else
	int r;
	sigset_t set;
	__block_app_sigs(&set);
	LOCK(t->killlock);
	if (!t->tid) {
		r = ESRCH;
	} else {
		r = -__syscall(SYS_sched_getparam, t->tid, param);
		if (!r) {
			*policy = __syscall(SYS_sched_getscheduler, t->tid);
		}
	}
	UNLOCK(t->killlock);
	__restore_sigs(&set);
	return r;
#endif
}
