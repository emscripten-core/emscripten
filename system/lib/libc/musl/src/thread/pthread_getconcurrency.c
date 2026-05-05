#include <pthread.h>

// XXX Emscripten marked as obsolescent in pthreads specification:
// http://pubs.opengroup.org/onlinepubs/9699919799/functions/pthread_getconcurrency.html
int pthread_getconcurrency()
{
	return 0;
}
