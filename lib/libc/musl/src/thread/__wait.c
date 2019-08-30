#ifdef __EMSCRIPTEN__
#include <math.h>
#endif

#include "pthread_impl.h"

int _pthread_isduecanceled(struct pthread *pthread_ptr);

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
	int is_main_thread = emscripten_is_main_runtime_thread();
	while (*addr==val) {
		if (pthread_self()->cancelasync == PTHREAD_CANCEL_ASYNCHRONOUS) {
			// Must wait in slices in case this thread is cancelled in between.
			int e;
			do {
				if (_pthread_isduecanceled(pthread_self())) {
					if (waiters) a_dec(waiters);
					return;
				}
				// Assist other threads by executing proxied operations that are effectively singlethreaded.
				if (is_main_thread) emscripten_main_thread_process_queued_calls();
				// Main thread waits in _very_ small slices so that it stays responsive to assist proxied
				// pthread calls.
				e = emscripten_futex_wait((void*)addr, val, is_main_thread ? 1 : 100);
			} while(e == -ETIMEDOUT);
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
