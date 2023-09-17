// Copyright 2015 The Emscripten Authors.  All rights reserved.
// Emscripten is available under two separate licenses, the MIT license and the
// University of Illinois/NCSA Open Source License.  Both these licenses can be
// found in the LICENSE file.

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <emscripten/em_asm.h>
#include <emscripten/console.h>
#include <assert.h>

#define NUM_THREADS 8

int fib(int n)
{
	if (n <= 0) return 0;
	if (n == 1) return 1;
	return fib(n-1) + fib(n-2);
}

unsigned int global_shared_data[NUM_THREADS];

void *ThreadMain(void *arg)
{
	long idx = (long)arg;
	unsigned int param = global_shared_data[idx];

#define N 100

	emscripten_errf("Thread idx %lu: sorting %d numbers with param %d", idx, N, param);

	unsigned int n[N];
	for(unsigned int i = 0; i < N; ++i)
		n[i] = (i + param) % N; // Create a shifted increasing sequence of numbers [0, N-1[

	// Sort the sequence to ordered [0, N[
	for(unsigned int i = 0; i < N; ++i)
		for(unsigned int j = i; j < N; ++j)
		{
			if (n[i] > n[j])
			{
				unsigned int t = n[i];
				n[i] = n[j];
				n[j] = t;
			}
		}
	// Ensure all elements are in place.
	int numGood = 0;
	for(unsigned int i = 0; i < N; ++i)
		if (n[i] == i) ++numGood;
		else emscripten_errf("n[%d]=%d", i, n[i]);

	emscripten_outf("Thread idx %ld with param %d: all done with result %d.", idx, param, numGood);
	pthread_exit((void*)numGood);
}

pthread_t thread[NUM_THREADS];

int numThreadsToCreate = 1000;

void CreateThread(int i)
{
	static int counter = 1;
	global_shared_data[i] = (counter++ * 12141231) & 0x7FFFFFFF; // Arbitrary random'ish data for perturbing the sort for this thread task.
//	emscripten_outf("Main: Creating thread idx %d (param %d)", i, global_shared_data[i]);
	int rc = pthread_create(&thread[i], NULL, ThreadMain, (void*)i);
	assert(rc == 0);
}

int main()
{
	// Create initial threads.
	for(int i = 0; i < NUM_THREADS; ++i)
		CreateThread(i);

	// Join all threads and create more.
	while (numThreadsToCreate > 0)
	{
		for(int i = 0; i < NUM_THREADS; ++i)
		{
			if (thread[i])
			{
				int status;
				int rc = pthread_join(thread[i], (void**)&status);
				assert(rc == 0);
				emscripten_errf("Main: Joined thread idx %d (param %d) with status %d", i, global_shared_data[i], (int)status);
				assert(status == N);
				thread[i] = 0;
				if (numThreadsToCreate > 0)
				{
					--numThreadsToCreate;
					CreateThread(i);
				}
			}
		}
	}
	printf("All threads joined.\n");
	return 0;
}
