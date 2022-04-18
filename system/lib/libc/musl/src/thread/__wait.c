#ifdef __EMSCRIPTEN__
#include <math.h>
#include <emscripten/threading.h>
#endif

#include "pthread_impl.h"

void __wait(volatile int *addr, volatile int *waiters, int val, int priv)
{
	int spins=100;
	if (priv) priv = FUTEX_PRIVATE;
	while (spins-- && (!waiters || !*waiters)) {
		if (*addr==val) a_spin();
		else return;
	}
	if (waiters) a_inc(waiters);
#ifdef __EMSCRIPTEN__
	int is_runtime_thread = emscripten_is_main_runtime_thread();

	// Main runtime thread may need to run proxied calls, so sleep in very small slices to be responsive.
	double max_ms_slice_to_sleep = is_runtime_thread ? 1 : 100;

	while (*addr==val) {
		if (is_runtime_thread || pthread_self()->cancelasync == PTHREAD_CANCEL_ASYNCHRONOUS) {
			int e;
			do {
				if (pthread_self()->cancel) {
					if (waiters) a_dec(waiters);
					return;
				}
				// Must wait in slices in case this thread is cancelled in between.
				e = emscripten_futex_wait((void*)addr, val, max_ms_slice_to_sleep);
			} while (e == -ETIMEDOUT);
		} else {
			// Can wait in one go.
			emscripten_futex_wait((void*)addr, val, INFINITY);
		}
	}
#else
	while (*addr==val) {
		__syscall(SYS_futex, addr, FUTEX_WAIT|priv, val, 0) != -ENOSYS
		|| __syscall(SYS_futex, addr, FUTEX_WAIT, val, 0);
	}
#endif
	if (waiters) a_dec(waiters);
}
