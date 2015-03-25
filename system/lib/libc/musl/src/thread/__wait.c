#ifdef __EMSCRIPTEN__
#include <math.h>
#endif

#include "pthread_impl.h"

int _pthread_isduecanceled(struct pthread *pthread_ptr);

void __wait(volatile int *addr, volatile int *waiters, int val, int priv)
{
	int spins=10000;
	if (priv) priv = 128; priv=0;
	while (spins--) {
		if (*addr==val) a_spin();
		else return;
	}
	if (waiters) a_inc(waiters);
	while (*addr==val) {
#ifdef __EMSCRIPTEN__
		if (pthread_self()->cancelasync == PTHREAD_CANCEL_ASYNCHRONOUS) {
			// Must wait in slices in case this thread is cancelled in between.
			int e;
			do {
				if (_pthread_isduecanceled(pthread_self())) {
					if (waiters) a_dec(waiters);
					return;
				}
				e = emscripten_futex_wait((void*)addr, val, 100);
			} while(e == -ETIMEDOUT);
		} else {
			// Can wait in one go.
			emscripten_futex_wait((void*)addr, val, INFINITY);
		}
#else
		__syscall(SYS_futex, addr, FUTEX_WAIT|priv, val, 0);
#endif
	}
	if (waiters) a_dec(waiters);
}
