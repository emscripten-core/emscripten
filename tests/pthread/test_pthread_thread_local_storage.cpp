#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <emscripten.h>
#include <emscripten/threading.h>
#include <errno.h>
#include <assert.h>
#include <inttypes.h>

#define NUM_THREADS 8
#define NUM_KEYS 16
#define NUM_ITERS 100

pthread_key_t keys[NUM_KEYS];
void *ThreadMain(void *arg)
{
	uintptr_t local_keys[NUM_KEYS];
	for(int iter = 0; iter < NUM_ITERS; ++iter)
	{
		for(int i = 0; i < NUM_KEYS; ++i)
		{
			local_keys[i] = (uintptr_t)pthread_getspecific(keys[i]);
//			EM_ASM_INT( { Module['printErr']('Thread ' + $0 + ': Read value ' + $1 + ' from TLS for key at index ' + $2); }, pthread_self(), (int)local_keys[i], i);
		}

		for(int i = 0; i < NUM_KEYS; ++i)
			++local_keys[i];

		for(int i = 0; i < NUM_KEYS; ++i)
			pthread_setspecific(keys[i], (void*)local_keys[i]);
	}

	for(int i = 0; i < NUM_KEYS; ++i)
	{
		local_keys[i] = (uintptr_t)pthread_getspecific(keys[i]);
//		EM_ASM_INT( { Module['printErr']('Thread ' + $0 + ' final verify: Read value ' + $1 + ' from TLS for key at index ' + $2); }, pthread_self(), (int)local_keys[i], i);
		if (local_keys[i] != NUM_ITERS)
			pthread_exit((void*)1);
	}
	pthread_exit(0);
}

pthread_t thread[NUM_THREADS];

int numThreadsToCreate = 32;

void CreateThread(int i)
{
	pthread_attr_t attr;
	pthread_attr_init(&attr);
	pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);
	int rc = pthread_create(&thread[i], &attr, ThreadMain, (void*)i);
	if (emscripten_has_threading_support()) assert(rc == 0);
	else assert(rc == EAGAIN);
	pthread_attr_destroy(&attr);
}

int main()
{
	for(int i = 0; i < NUM_KEYS; ++i)
		pthread_key_create(&keys[i], NULL);

	// Create initial threads.
	for(int i = 0; i < NUM_THREADS; ++i)
		CreateThread(i);

	// Join all threads and create more.
	if (emscripten_has_threading_support())
	{
		for(int i = 0; i < NUM_THREADS; ++i)
		{
			if (thread[i])
			{
				int status;
				int rc = pthread_join(thread[i], (void**)&status);
				assert(rc == 0);
				EM_ASM_INT( { Module['printErr']('Main: Joined thread idx ' + $0 + ' with status ' + $1); }, i, (int)status);
				assert(status == 0);
				thread[i] = 0;
				if (numThreadsToCreate > 0)
				{
					--numThreadsToCreate;
					CreateThread(i);
				}
			}
		}
	}
#ifdef REPORT_RESULT
	int result = 0;
	REPORT_RESULT();
#endif

	for(int i = 0; i < NUM_KEYS; ++i)
		pthread_key_delete(keys[i]);
}
