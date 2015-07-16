#include <aio.h>
#include <errno.h>
#include "pthread_impl.h"

/* Due to the requirement that aio_suspend be async-signal-safe, we cannot
 * use any locks, wait queues, etc. that would make it more efficient. The
 * only obviously-correct algorithm is to generate a wakeup every time any
 * aio operation finishes and have aio_suspend re-evaluate the completion
 * status of each aiocb it was waiting on. */

static volatile int seq;

void __aio_wake(void)
{
	a_inc(&seq);
	__wake(&seq, -1, 1);
}

int aio_suspend(const struct aiocb *const cbs[], int cnt, const struct timespec *ts)
{
	int i, last, first=1, ret=0;
	struct timespec at;

	if (cnt<0) {
		errno = EINVAL;
		return -1;
	}

	for (;;) {
		last = seq;

		for (i=0; i<cnt; i++) {
			if (cbs[i] && cbs[i]->__err != EINPROGRESS)
				return 0;
		}

		if (first && ts) {
			clock_gettime(CLOCK_MONOTONIC, &at);
			at.tv_sec += ts->tv_sec;
			if ((at.tv_nsec += ts->tv_nsec) >= 1000000000) {
				at.tv_nsec -= 1000000000;
				at.tv_sec++;
			}
			first = 0;
		}

		ret = __timedwait(&seq, last, CLOCK_MONOTONIC,
			ts ? &at : 0, 0, 0, 1);

		if (ret == ETIMEDOUT) ret = EAGAIN;

		if (ret) {
			errno = ret;
			return -1;
		}
	}
}
