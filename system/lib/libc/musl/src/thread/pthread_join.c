#define _GNU_SOURCE
#include "pthread_impl.h"
#include <sys/mman.h>

static void dummy1(pthread_t t)
{
}
weak_alias(dummy1, __tl_sync);

static int __pthread_timedjoin_np(pthread_t t, void **res, const struct timespec *at)
{
#ifdef __EMSCRIPTEN__
	// Attempt to join a thread which does not point to a valid thread, or
	// does not exist anymore.
	if (!_emscripten_thread_is_valid(t)) return ESRCH;
	// Thread is attempting to join to itself.  Already detached threads are
	// handled below by returning EINVAL instead.
	// TODO: The detached check here is just to satisfy the
	// `other.test_{proxy,main}_pthread_join_detach` tests.
	if (t->detach_state != DT_DETACHED && __pthread_self() == t) return EDEADLK;
#endif
	int state, cs, r = 0;
	__pthread_testcancel();
	__pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, &cs);
	if (cs == PTHREAD_CANCEL_ENABLE) __pthread_setcancelstate(cs, 0);
	while ((state = t->detach_state) && r != ETIMEDOUT && r != EINVAL) {
#ifdef __EMSCRIPTEN__
		// The thread is (already) detached and therefore not joinable.
		// This also handle cases where the thread becomes detached
		// *during* the join.
		if (state >= DT_DETACHED) {
			// Even though the man page says this is undefined behaviour we have
			// several tests in the posixtest suite that depend on this.
			r = EINVAL;
			break;
		}
#else
		if (state >= DT_DETACHED) a_crash();
#endif
		r = __timedwait_cp(&t->detach_state, state, CLOCK_REALTIME, at, 1);
	}
	__pthread_setcancelstate(cs, 0);
	if (r == ETIMEDOUT || r == EINVAL) return r;
	__tl_sync(t);
	if (res) *res = t->result;
#ifdef __EMSCRIPTEN__
	// Thread was exited during this call, be sure to clean it up.
	if (state == DT_EXITED) _emscripten_thread_cleanup(t);
#else // XXX Emscripten map_base unused
	if (t->map_base) __munmap(t->map_base, t->map_size);
#endif
	return 0;
}

int __pthread_join(pthread_t t, void **res)
{
#ifdef __EMSCRIPTEN__ // XXX Emscripten check whether blocking is allowed.
	emscripten_check_blocking_allowed();
#endif
	return __pthread_timedjoin_np(t, res, 0);
}

static int __pthread_tryjoin_np(pthread_t t, void **res)
{
#ifdef __EMSCRIPTEN__ // XXX Emscripten call __pthread_timedjoin_np directly to avoid additional check
	return t->detach_state==DT_JOINABLE ? EBUSY : __pthread_timedjoin_np(t, res, 0);
#else
	return t->detach_state==DT_JOINABLE ? EBUSY : __pthread_join(t, res);
#endif
}

weak_alias(__pthread_tryjoin_np, pthread_tryjoin_np);
weak_alias(__pthread_timedjoin_np, pthread_timedjoin_np);
weak_alias(__pthread_join, pthread_join);
#ifdef __EMSCRIPTEN__ // XXX Emscripten add an extra alias for LSan
weak_alias(__pthread_join, emscripten_builtin_pthread_join);
#endif
