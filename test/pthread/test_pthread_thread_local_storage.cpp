// Copyright 2015 The Emscripten Authors.  All rights reserved.
// Emscripten is available under two separate licenses, the MIT license and the
// University of Illinois/NCSA Open Source License.  Both these licenses can be
// found in the LICENSE file.

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
//		emscripten_errf("Thread %d: Read value %d from TLS for key at index %d", pthread_self(), (int)local_keys[i], i);
		}

		for(int i = 0; i < NUM_KEYS; ++i)
			++local_keys[i];

		for(int i = 0; i < NUM_KEYS; ++i)
			pthread_setspecific(keys[i], (void*)local_keys[i]);
	}

	for(int i = 0; i < NUM_KEYS; ++i)
	{
		local_keys[i] = (uintptr_t)pthread_getspecific(keys[i]);
//	emscripten_errf("Thread %d final verify: Read value %d from TLS for key at index %d", pthread_self(), (int)local_keys[i], i);
		assert(local_keys[i] == NUM_ITERS);
	}
	return 0;
}

pthread_t thread[NUM_THREADS];

int numThreadsToCreate = 32;
int threadCounter = 0;
_Atomic int destructorCounter = 0;

void CreateThread(long i)
{
	printf("CreateThread %ld\n", i);
	threadCounter++;
	int rc = pthread_create(&thread[i], NULL, ThreadMain, (void*)i);
	if (emscripten_has_threading_support()) assert(rc == 0);
	else assert(rc == EAGAIN);
}

void destructor1(void* val) {
	destructorCounter++;
}

int main()
{
	for(int i = 0; i < NUM_KEYS; ++i) {
		if (i == 0)
			pthread_key_create(&keys[i], destructor1);
		else
			pthread_key_create(&keys[i], NULL);
	}

	// Create initial threads.
	for(long i = 0; i < NUM_THREADS; ++i)
		CreateThread(i);

	// Join all threads and create more.
	if (emscripten_has_threading_support())
	{
		for(long i = 0; i < NUM_THREADS; ++i)
		{
			if (thread[i])
			{
				intptr_t status;
				int rc = pthread_join(thread[i], (void**)&status);
				assert(rc == 0);
				printf("Main: Joined thread idx %ld with status %lu\n", i, status);
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

	for(int i = 0; i < NUM_THREADS; ++i)
	{
			if (thread[i])
			{
				intptr_t status = 1;
				int rc = pthread_join(thread[i], (void**)&status);
				assert(rc == 0);
				printf("Main: Joined thread idx %d with status %lu\n", i, status);
				assert(status == 0);
			}
	}
	printf("destructorCounter: %d\n", destructorCounter);
	printf("threadCounter: %d\n", threadCounter);
	assert(destructorCounter == threadCounter);

	for(int i = 0; i < NUM_KEYS; ++i)
		pthread_key_delete(keys[i]);

	printf("done\n");
	return 0;
}
