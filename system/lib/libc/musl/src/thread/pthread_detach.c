#include "pthread_impl.h"
#include <threads.h>

static int __pthread_detach(pthread_t t)
{
	// XXX EMSCRIPTEN: Add check for invalid (already joined) thread.  Again
	// for the benefit of the conformance tests.
	if (t->self != t)
		return ESRCH;
	/* If the cas fails, detach state is either already-detached
	 * or exiting/exited, and pthread_join will trap or cleanup. */
	if (a_cas(&t->detach_state, DT_JOINABLE, DT_DETACHED) != DT_JOINABLE)
		return __pthread_join(t, 0);
	return 0;
}

weak_alias(__pthread_detach, pthread_detach);
weak_alias(__pthread_detach, thrd_detach);
// XXX EMSCRIPTEN: add extra alias for asan.
weak_alias(__pthread_detach, emscripten_builtin_pthread_detach);
