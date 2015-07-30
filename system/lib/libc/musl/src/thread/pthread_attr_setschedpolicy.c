#include "pthread_impl.h"

int pthread_attr_setschedpolicy(pthread_attr_t *a, int policy)
{
#ifdef __EMSCRIPTEN__ // XXX Emscripten: upstream this fix to musl.
	if (policy < SCHED_OTHER || (policy & ~SCHED_RESET_ON_FORK) > SCHED_IDLE) return EINVAL;
#endif
	a->_a_policy = policy;
	return 0;
}
