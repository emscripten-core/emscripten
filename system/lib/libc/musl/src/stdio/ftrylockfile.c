#include "stdio_impl.h"
#include "pthread_impl.h"
#include <limits.h>

int ftrylockfile(FILE *f)
{
	int tid = pthread_self()->tid;
	if (f->lock == tid) {
		if (f->lockcount == LONG_MAX)
			return -1;
		f->lockcount++;
		return 0;
	}
	if (f->lock < 0) f->lock = 0;
	if (f->lock || a_cas(&f->lock, 0, tid))
		return -1;
	f->lockcount = 1;
	return 0;
}
