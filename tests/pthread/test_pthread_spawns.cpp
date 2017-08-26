#include <pthread.h>
#include <emscripten/threading.h>

#define NUM_THREADS 2

void *thread_main(void *arg)
{
	pthread_exit(0);
}

pthread_t thread[NUM_THREADS];

int main()
{
	for(int x = 0; x < 100; ++x)
	{
		for(int i = 0; i < NUM_THREADS; ++i) pthread_create(&thread[i], NULL, thread_main, 0);
		if (emscripten_has_threading_support())
			for(int i = 0; i < NUM_THREADS; ++i) pthread_join(thread[i], NULL);
	}
#ifdef REPORT_RESULT
	REPORT_RESULT(0);
#endif
}
