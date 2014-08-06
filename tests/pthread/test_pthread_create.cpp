#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <emscripten.h>
#include <assert.h>

#define NUM_THREADS 8

int fib(int n)
{
	if (n <= 0) return 0;
	if (n == 1) return 1;
	return fib(n-1) + fib(n-2);
}

void *ThreadMain(void *arg)
{
	unsigned int param = *(unsigned int*)arg; // param == random-generated integer in main thread.
	param &= 0x7FFFFFFF; // Arbitrary, for printing convenience.

#define N 100

	EM_ASM_INT( { Module['printErr']('Thread ID '+$0+': sorting ' + $1 + ' numbers.') }, param, N);

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
		else EM_ASM_INT( { Module['printErr']('n['+$0+']='+$1); }, i, n[i]);

	pthread_exit((void*)numGood);
}

int global_shared_data[NUM_THREADS];
pthread_t thread[NUM_THREADS];

int numThreadsToCreate = 1000;

void CreateThread(int i)
{
	pthread_attr_t attr;
	pthread_attr_init(&attr);
	pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);
	global_shared_data[i] = numThreadsToCreate * 12141231;
	int rc = pthread_create(&thread[i], &attr, ThreadMain, &global_shared_data[i]);
	assert(rc == 0);
	pthread_attr_destroy(&attr);
}

int main()
{
	malloc(4); // Work around bug https://github.com/kripken/emscripten/issues/2621

	// Create initial threads.
	for(int i = 0; i < NUM_THREADS; ++i)
		CreateThread(i);

	// Join all threads and create more.
	for(int i = 0; i < NUM_THREADS; ++i)
	{
		if (thread[i])
		{
			int status;
			int rc = pthread_join(thread[i], (void**)&status);
			assert(rc == 0);
			EM_ASM_INT( { Module['printErr']('Main: Joined thread ' + $0 + ' with status ' + $1); }, thread[i], (int)status);
			assert(status == N);
			thread[i] = 0;
			if (numThreadsToCreate > 0)
			{
				--numThreadsToCreate;
				CreateThread(i);
			}
		}
	}
#ifdef REPORT_RESULT
	int result = 0;
	REPORT_RESULT();
#endif
}
