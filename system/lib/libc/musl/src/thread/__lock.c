#include "pthread_impl.h"

void __lock(volatile int *l)
{
	if (libc.threads_minus_1)
		while (a_swap(l, 1)) __wait(l, l+1, 1, 1);
}

void __unlock(volatile int *l)
{
	if (l[0]) {
		a_store(l, 0);
		if (l[1]) __wake(l, 1, 1);
	}
}
