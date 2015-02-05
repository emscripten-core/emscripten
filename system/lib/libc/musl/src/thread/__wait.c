#ifdef __EMSCRIPTEN__
#include <math.h>
#endif

#include "pthread_impl.h"

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
		emscripten_futex_wait((void*)addr, val, INFINITY);
#else
		__syscall(SYS_futex, addr, FUTEX_WAIT|priv, val, 0);
#endif
	}
	if (waiters) a_dec(waiters);
}
