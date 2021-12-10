#ifdef __EMSCRIPTEN__
#include <math.h>
#include <emscripten/threading.h>
#endif

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
	const int is_runtime_thread = emscripten_is_main_runtime_thread();

	// Main runtime thread may need to run proxied calls, so sleep in very small slices to be responsive.
#ifdef __EMSCRIPTEN_ATOMIC_BUILTINS__
	const int64_t maxNsecsToSleep = is_runtime_thread ? 1000000 : 100000000;
#else // !defined(__EMSCRIPTEN_ATOMIC_BUILTINS__)
	const double maxMsecsToSleep = is_runtime_thread ? 1 : 100;
#endif // defined(__EMSCRIPTEN_ATOMIC_BUILTINS__)

	while (*addr==val) {
		if (is_runtime_thread || self->cancelasync) {
			int e;
			do {
				if (self->cancel) {
					if (waiters) a_dec(waiters);
					return;
				}
				// Must wait in slices in case this thread is cancelled in between.
#ifdef __EMSCRIPTEN_ATOMIC_BUILTINS__
				e = __builtin_wasm_memory_atomic_wait32((int*)addr, val, maxNsecsToSleep);
				if (e == 2) e = -ETIMEDOUT;
#else // !defined(__EMSCRIPTEN_ATOMIC_BUILTINS__)
				e = emscripten_futex_wait((void*)addr, val, maxMsecsToSleep);
#endif // defined(__EMSCRIPTEN_ATOMIC_BUILTINS__)
				// Assist other threads by executing proxied operations that are effectively singlethreaded.
				if (is_runtime_thread) emscripten_main_thread_process_queued_calls();
			} while (e == -ETIMEDOUT);
		} else {
			// Can wait in one go.
#ifdef __EMSCRIPTEN_ATOMIC_BUILTINS__
			__builtin_wasm_memory_atomic_wait32((int*)addr, val, -1);
#else // !defined(__EMSCRIPTEN_ATOMIC_BUILTINS__)
			emscripten_futex_wait((void*)addr, val, INFINITY);
#endif // defined(__EMSCRIPTEN_ATOMIC_BUILTINS__)
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
