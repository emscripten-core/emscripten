#define _GNU_SOURCE
#include "pthread_impl.h"
#include <sys/mman.h>

static void dummy1(pthread_t t)
{
}
weak_alias(dummy1, __tl_sync);

static int __pthread_timedjoin_np(pthread_t t, void **res, const struct timespec *at)
{
#ifdef __EMSCRIPTEN__ // XXX Emscripten return ESRCH when attempting to join invalid threads (see test_pthread_join_6_2)
	if (!t || !t->tid) return ESRCH;
#endif

	int state, cs, r = 0;
	__pthread_testcancel();
	__pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, &cs);
	if (cs == PTHREAD_CANCEL_ENABLE) __pthread_setcancelstate(cs, 0);
	while ((state = t->detach_state) && r != ETIMEDOUT && r != EINVAL) {
#ifdef __EMSCRIPTEN__
		// XXX Emscripten return EINVAL when attempting to join an already detached thread
		if (state >= DT_DETACHED) {
			r = EINVAL;
			break;
		}
		// XXX Emscripten we need to check if blocking is allowed when the initially state is joinable.
		// Note that detached and exiting/exited threads can be joined without blocking.
		if (r == 0 && state == DT_JOINABLE)
			emscripten_check_blocking_allowed();
#else
		if (state >= DT_DETACHED) a_crash();
#endif
		r = __timedwait_cp(&t->detach_state, state, CLOCK_REALTIME, at, 1);
	}
	__pthread_setcancelstate(cs, 0);
	if (r == ETIMEDOUT || r == EINVAL) return r;
	__tl_sync(t);
	if (res) *res = t->result;
#ifdef __EMSCRIPTEN__ // XXX Emscripten cleanup thread
	if (state == DT_EXITED) emscripten_cleanup_thread(t);
#else // XXX Emscripten map_base unused
	if (t->map_base) __munmap(t->map_base, t->map_size);
#endif
	return 0;
}

int __pthread_join(pthread_t t, void **res)
{
	return __pthread_timedjoin_np(t, res, 0);
}

static int __pthread_tryjoin_np(pthread_t t, void **res)
{
	return t->detach_state==DT_JOINABLE ? EBUSY : __pthread_join(t, res);
}

weak_alias(__pthread_tryjoin_np, pthread_tryjoin_np);
weak_alias(__pthread_timedjoin_np, pthread_timedjoin_np);
weak_alias(__pthread_join, pthread_join);

#ifdef __EMSCRIPTEN__ // XXX Emscripten needed by LSan
weak_alias(__pthread_join, emscripten_builtin_pthread_join);
#endif
