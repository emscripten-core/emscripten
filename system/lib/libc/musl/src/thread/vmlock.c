#include "pthread_impl.h"

static int vmlock[2];

void __vm_lock(int inc)
{
	for (;;) {
		int v = vmlock[0];
		if (inc*v < 0) __wait(vmlock, vmlock+1, v, 1);
		else if (a_cas(vmlock, v, v+inc)==v) break;
	}
}

void __vm_unlock(void)
{
	int inc = vmlock[0]>0 ? -1 : 1;
	if (a_fetch_add(vmlock, inc)==-inc && vmlock[1])
		__wake(vmlock, -1, 1);
}

weak_alias(__vm_lock, __vm_lock_impl);
weak_alias(__vm_unlock, __vm_unlock_impl);
