#include "pthread_impl.h"

void __wait(volatile int *addr, volatile int *waiters, int val, int priv)
{
	int spins=100;
#ifndef __EMSCRIPTEN__
	if (priv) priv = FUTEX_PRIVATE;
#endif
	while (spins-- && (!waiters || !*waiters)) {
		if (*addr==val) a_spin();
		else return;
	}
	if (waiters) a_inc(waiters);
#ifdef __EMSCRIPTEN__
	pthread_t self = __pthread_self();
	int is_main_thread = emscripten_is_main_runtime_thread();

	// Main runtime thread may need to run proxied calls, so sleep in very small slices to be responsive.
	const double maxMsecsSliceToSleep = is_main_thread ? 1 : 100;

	while (*addr==val) {
		if (self->cancelasync) {
			// Must wait in slices in case this thread is cancelled in between.
			int e;
			do {
				if (self->cancel) {
					if (waiters) a_dec(waiters);
					return;
				}
				// Assist other threads by executing proxied operations that are effectively singlethreaded.
				if (is_main_thread) emscripten_main_thread_process_queued_calls();
				e = emscripten_futex_wait(addr, val, maxMsecsSliceToSleep);
			} while(e == -ETIMEDOUT);
		} else {
			// Can wait in one go.
			emscripten_futex_wait(addr, val, INFINITY);
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
