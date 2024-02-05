#include <pthread.h>
#include <errno.h>

int pthread_setconcurrency(int val)
{
#ifndef __EMSCRIPTEN__ // XXX Emscripten marked as obsolescent in pthreads specification
	if (val < 0) return EINVAL;
	if (val > 0) return EAGAIN;
#endif
	return 0;
}
