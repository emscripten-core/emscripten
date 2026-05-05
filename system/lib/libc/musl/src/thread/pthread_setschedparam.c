#include "pthread_impl.h"
#include "lock.h"

int pthread_setschedparam(pthread_t t, int policy, const struct sched_param *param)
{
#ifdef __EMSCRIPTEN__ // XXX Emscripten web or Node workers doesn't support prioritizing threads
	// no-op
	return 0;
#else
	int r;
	sigset_t set;
	__block_app_sigs(&set);
	LOCK(t->killlock);
	r = !t->tid ? ESRCH : -__syscall(SYS_sched_setscheduler, t->tid, policy, param);
	UNLOCK(t->killlock);
	__restore_sigs(&set);
	return r;
#endif
}
