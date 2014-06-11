#include <time.h>
#include <sys/time.h>
#include "syscall.h"

int __clock_gettime(clockid_t, struct timespec *);

time_t time(time_t *t)
{
	struct timespec ts;
	__clock_gettime(CLOCK_REALTIME, &ts);
	if (t) *t = ts.tv_sec;
	return ts.tv_sec;
}
