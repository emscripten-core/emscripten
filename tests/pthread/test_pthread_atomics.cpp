#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <pthread.h>
#include <emscripten.h>
#include <emscripten/threading.h>
#include <assert.h>

#define NUM_THREADS 8

const int N = 10;
int sharedData[N] = {};

struct Test
{
	int op;
	int threadId;
};

void *ThreadMain(void *arg)
{
	assert(pthread_self() != 0);
	struct Test *t = (struct Test*)arg;
	EM_ASM_INT( { Module['print']('Thread ' + $0 + ' for test ' + $1 + ': starting computation.'); }, t->threadId, t->op);

	for(int i = 0; i < 99999; ++i)
		for(int j = 0; j < N; ++j)
		{
			switch(t->op)
			{
				case 0: emscripten_atomic_add_u32(&sharedData[j], 1); break;
				case 1: emscripten_atomic_sub_u32(&sharedData[j], 1); break;
				case 2: emscripten_atomic_and_u32(&sharedData[j], ~(1UL << t->threadId)); break;
				case 3: emscripten_atomic_or_u32(&sharedData[j], 1UL << t->threadId); break;
				case 4: emscripten_atomic_xor_u32(&sharedData[j], 1UL << t->threadId); break;
			}
		}
	EM_ASM_INT( { Module['print']('Thread ' + $0 + ' for test ' + $1 + ': finished, exit()ing.'); }, t->threadId, t->op);
	pthread_exit(0);
}

struct Test t[NUM_THREADS] = {};
pthread_t thread[NUM_THREADS];

void RunTest(int test)
{	
	pthread_attr_t attr;
	pthread_attr_init(&attr);
	pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);
	pthread_attr_setstacksize(&attr, 4*1024);

	printf("Main thread has thread ID %d\n", (int)pthread_self());
	assert(pthread_self() != 0);

	memset(sharedData, (test == 2) ? 0xFF : 0, sizeof(sharedData));

	EM_ASM_INT( { Module['print']('Main: Starting test ' + $0); }, test);

	for(int i = 0; i < NUM_THREADS; ++i)
	{
		t[i].op = test;
		t[i].threadId = i;
		int rc = pthread_create(&thread[i], &attr, ThreadMain, &t[i]);
		assert(rc == 0);
	}

	pthread_attr_destroy(&attr);

	for(int i = 0; i < NUM_THREADS; ++i)
	{
		int status = 1;
		int rc = pthread_join(thread[i], (void**)&status);
		assert(rc == 0);
		assert(status == 0);
	}

	int val = sharedData[0];
	EM_ASM_INT( { Module['print']('Main: Test ' + $0 + ' finished. Result: ' + $1); }, test, val);
	for(int i = 1; i < N; ++i)
		assert(sharedData[i] == sharedData[0]);
}

int main()
{
	malloc(4); // Work around bug https://github.com/kripken/emscripten/issues/2621

	for(int i = 0; i < 6; ++i)
		RunTest(i);

#ifdef REPORT_RESULT
	int result = 0;
	REPORT_RESULT();
#endif
}
