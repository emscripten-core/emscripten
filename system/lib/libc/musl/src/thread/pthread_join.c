#include "pthread_impl.h"
#include <sys/mman.h>

static void dummy(void *p)
{
}

int pthread_join(pthread_t t, void **res)
{
	int tmp;
	while ((tmp = t->tid)) __timedwait(&t->tid, tmp, 0, 0, dummy, 0, 0);
	if (res) *res = t->result;
	if (t->map_base) munmap(t->map_base, t->map_size);
	return 0;
}
