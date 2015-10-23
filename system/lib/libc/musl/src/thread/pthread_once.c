#include "pthread_impl.h"

static void undo(void *control)
{
	a_store(control, 0);
	__wake(control, 1, 0);
}

int pthread_once(pthread_once_t *control, void (*init)(void))
{
	static int waiters;

	/* Return immediately if init finished before, but ensure that
	 * effects of the init routine are visible to the caller. */
	if (*control == 2) {
		/* Otherwise-useless cas just to get a barrier. */
		a_cas(&(int){0},0,0);
		return 0;
	}

	/* Try to enter initializing state. Three possibilities:
	 *  0 - we're the first or the other cancelled; run init
	 *  1 - another thread is running init; wait
	 *  2 - another thread finished running init; just return */

	for (;;) switch (a_cas(control, 0, 1)) {
	case 0:
		pthread_cleanup_push(undo, control);
		init();
		pthread_cleanup_pop(0);

		a_store(control, 2);
		if (waiters) __wake(control, -1, 0);
		return 0;
	case 1:
		__wait(control, &waiters, 1, 0);
		continue;
	case 2:
		return 0;
	}
}
