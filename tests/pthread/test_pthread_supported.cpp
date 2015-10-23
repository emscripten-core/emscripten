#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <emscripten/emscripten.h>
#include <emscripten/threading.h>
#include <assert.h>
#include <errno.h>

void *ThreadMain(void *arg)
{
	pthread_exit(0);
}

int main()
{
	pthread_t thread;
	pthread_attr_t attr;
	pthread_attr_init(&attr);
	pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);
	int rc = pthread_create(&thread, &attr, ThreadMain, 0);
	pthread_attr_destroy(&attr);
	pthread_join(thread, 0);

	if (emscripten_has_threading_support()) assert(rc == 0);
	else assert(rc == EAGAIN);

#ifdef REPORT_RESULT
	int result = 0;
	REPORT_RESULT();
#endif
}
