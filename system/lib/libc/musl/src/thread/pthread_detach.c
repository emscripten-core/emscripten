#include "pthread_impl.h"
#include <threads.h>

static int __pthread_detach(pthread_t t)
{
#ifdef __EMSCRIPTEN__
	// XXX Emscripten: Add check for invalid (already joined) thread.  Again
	// for the benefit of the conformance tests.
	if (!_emscripten_thread_is_valid(t)) return ESRCH;

	// Note that we don't use pthread_join here, to avoid
	// returning EDEADLK when attempting to detach itself.
	switch (a_cas(&t->detach_state, DT_JOINABLE, DT_DETACHED)) {
	case DT_EXITED:
	case DT_EXITING:
		_emscripten_thread_cleanup(t);
		return 0;
	case DT_JOINABLE:
		return 0;
	case DT_DETACHED: // already-detached
	default: // >= DT_DETACHED
		// Even though the man page says this is undefined behaviour to attempt to
		// detach an already-detached thread we have several tests in the posixtest
		// suite that depend on this (pthread_join.c)
		return EINVAL;
	}
#else
	/* If the cas fails, detach state is either already-detached
	 * or exiting/exited, and pthread_join will trap or cleanup. */
	if (a_cas(&t->detach_state, DT_JOINABLE, DT_DETACHED) != DT_JOINABLE) {
		int cs;
		__pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, &cs);
		__pthread_join(t, 0);
		__pthread_setcancelstate(cs, 0);
	}
	return 0;
#endif
}

weak_alias(__pthread_detach, pthread_detach);
weak_alias(__pthread_detach, thrd_detach);
#ifdef __EMSCRIPTEN__ // XXX Emscripten add an extra alias for ASan/LSan.
weak_alias(__pthread_detach, emscripten_builtin_pthread_detach);
#endif
