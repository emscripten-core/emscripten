#include "pthread_impl.h"
#include <sys/mman.h>

int __munmap(void *, size_t);
void pthread_testcancel(void);
int pthread_setcancelstate(int, int *);

int __pthread_timedjoin_np(pthread_t t, void **res, const struct timespec *at)
{
	int tmp, cs, r = 0;
	pthread_testcancel();
	pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, &cs);
	if (cs == PTHREAD_CANCEL_ENABLE) pthread_setcancelstate(cs, 0);
	while ((tmp = t->tid) && r != ETIMEDOUT && r != EINVAL)
		r = __timedwait_cp(&t->tid, tmp, CLOCK_REALTIME, at, 0);
	pthread_setcancelstate(cs, 0);
	if (r == ETIMEDOUT || r == EINVAL) return r;
	a_barrier();
	if (res) *res = t->result;
	if (t->map_base) __munmap(t->map_base, t->map_size);
	return 0;
}

int __pthread_join(pthread_t t, void **res)
{
	return __pthread_timedjoin_np(t, res, 0);
}

int __pthread_tryjoin_np(pthread_t t, void **res)
{
	return t->tid ? EBUSY : __pthread_join(t, res);
}

weak_alias(__pthread_tryjoin_np, pthread_tryjoin_np);
weak_alias(__pthread_timedjoin_np, pthread_timedjoin_np);
weak_alias(__pthread_join, pthread_join);
