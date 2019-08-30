#include <threads.h>
#include <errno.h>

int __pthread_mutex_timedlock(mtx_t *restrict, const struct timespec *restrict);

int mtx_timedlock(mtx_t *restrict m, const struct timespec *restrict ts)
{
	int ret = __pthread_mutex_timedlock(m, ts);
	switch (ret) {
	default:        return thrd_error;
	case 0:         return thrd_success;
	case ETIMEDOUT: return thrd_timedout;
	}
}
