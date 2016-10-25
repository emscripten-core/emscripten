#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <emscripten/threading.h>
#include <assert.h>

unsigned int main_thread_wait_val = 1;
int result = 1;

void *ThreadMain(void *arg)
{
	for(int i = 0; i < 10; ++i)
	{
		char str[256];
		sprintf(str, "file%d.txt", i);
		printf("Writing file %s..\n", str); // Prints out to page console, this is a proxied operation.
		FILE *handle = fopen(str, "w"); // fopen, fputs and fclose are currently proxied operations too (although hopefully not in the future)
		fputs(str, handle);
		fclose(handle);
	}
	emscripten_atomic_store_u32(&main_thread_wait_val, 0);
	emscripten_futex_wake(&main_thread_wait_val, 1);
	emscripten_atomic_store_u32(&result, 0);
	pthread_exit(0);
}

int main()
{
	if (!emscripten_has_threading_support())
	{
#ifdef REPORT_RESULT
		result = 0;
		REPORT_RESULT();
#endif
		printf("Skipped: Threading is not supported.\n");
		return 0;
	}

	pthread_t thread;
	pthread_attr_t attr;
	pthread_attr_init(&attr);
	pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);
	int rc = pthread_create(&thread, &attr, ThreadMain, 0);
	assert(rc == 0);
	rc = emscripten_futex_wait(&main_thread_wait_val, 1, 15 * 1000);
	if (rc != 0)
	{
		printf("ERROR! futex wait timed out!\n");
		result = 2;
#ifdef REPORT_RESULT
		REPORT_RESULT();
#endif
		exit(1);
	}
	pthread_attr_destroy(&attr);
	pthread_join(thread, 0);		

#ifdef REPORT_RESULT
	REPORT_RESULT();
#endif
}
