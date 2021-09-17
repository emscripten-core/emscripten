#include "pthread_impl.h"
#include <threads.h>

int __pthread_join(pthread_t, void **);

static int __pthread_detach(pthread_t t)
{
	// XXX EMSCRIPTEN: Add check for invalid (already joined) thread.  Again
	// for the benefit of the conformance tests.
	if (t->self != t)
		return ESRCH;
	// XXX EMSCRIPTEN: Even though the man page says this is undefined behaviour
	// we have several tests in the posixtest suite that depend on this.
	if (t->detached)
		return EINVAL;
	/* Cannot detach a thread that's already exiting */
	if (a_swap(t->exitlock, 1))
		return __pthread_join(t, 0);
	t->detached = 2;
	__unlock(t->exitlock);
	return 0;
}

weak_alias(__pthread_detach, pthread_detach);
weak_alias(__pthread_detach, thrd_detach);
// XXX EMSCRIPTEN: add extra alias for asan.
weak_alias(__pthread_detach, emscripten_builtin_pthread_detach);
