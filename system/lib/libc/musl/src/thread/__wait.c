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
	while (*addr==val)
		__syscall(SYS_futex, addr, FUTEX_WAIT|priv, val, 0);
	if (waiters) a_dec(waiters);
}
