#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <emscripten.h>
#include <emscripten/threading.h>
#include <assert.h>
#include <iostream>

void *ThreadMain(void *arg)
{
	std::cout << "Hello from thread" << std::endl;

	return 0;
}

int numThreadsToCreate = 1000;

int main()
{
  int result = 0;
  if (!emscripten_has_threading_support())
  {
#ifdef REPORT_RESULT
    REPORT_RESULT();
#endif
    printf("Skipped: Threading is not supported.\n");
    return 0;
  }

	pthread_t thread;
	int rc = pthread_create(&thread, NULL, ThreadMain, 0);
	assert(rc == 0);

	rc = pthread_join(thread, NULL);
	assert(rc == 0);

	std::cout << "The thread should print 'Hello from thread'" << std::endl;

#ifdef REPORT_RESULT
	REPORT_RESULT();
#endif
}
