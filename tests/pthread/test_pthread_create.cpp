#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <emscripten.h>
#include <emscripten/threading.h>
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
	int idx = (int)arg;
	unsigned int param = global_shared_data[idx];

#define N 100

	EM_ASM_INT( { Module['printErr']('Thread idx '+$0+': sorting ' + $1 + ' numbers with param ' + $2 + '.') }, idx, N, param);

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

	EM_ASM_INT( { Module['print']('Thread idx ' + $0 + ' with param '+$1+': all done with result '+$2+'.'); }, idx, param, numGood);
	pthread_exit((void*)numGood);
}

pthread_t thread[NUM_THREADS];

int numThreadsToCreate = 1000;

void CreateThread(int i)
{
	pthread_attr_t attr;
	pthread_attr_init(&attr);
	pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);
	static int counter = 1;
	global_shared_data[i] = (counter++ * 12141231) & 0x7FFFFFFF; // Arbitrary random'ish data for perturbing the sort for this thread task.
//	EM_ASM_INT( { Module['print']('Main: Creating thread idx ' + $0 + ' (param ' + $1 + ')'); }, i, global_shared_data[i]);
	int rc = pthread_create(&thread[i], &attr, ThreadMain, (void*)i);
	assert(rc == 0);
	pthread_attr_destroy(&attr);
}

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
			EM_ASM_INT( { Module['printErr']('Main: Joined thread idx ' + $0 + ' (param ' + $1 + ') with status ' + $2); }, i, global_shared_data[i], (int)status);
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
	REPORT_RESULT();
#endif
}
