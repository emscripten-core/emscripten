#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <pthread.h>
#include <emscripten.h>
#include <emscripten/threading.h>
#include <assert.h>

#define NUM_THREADS 8

volatile double globalDouble = 0.0;
volatile uint64_t globalU64 = 0;

const int N = 10;
uint64_t sharedData[N] = {};

struct Test
{
	int op;
	int threadId;
};

uint64_t threadCasAccumulatedWrittenData[NUM_THREADS] = {};
uint64_t threadCasAccumulatedReadData[NUM_THREADS] = {};

int64_t rand_60()
{
	return (int64_t)(emscripten_random() * 0x3FFFFFFF) | ((int64_t)(emscripten_random() * 0x3FFFFFFF) << 30);
}

void *ThreadMain(void *arg)
{
	assert(pthread_self() != 0);
	assert(globalDouble == 5.0);
	assert(globalU64 == 5);
	struct Test *t = (struct Test*)arg;
	EM_ASM_INT( { Module['print']('Thread ' + $0 + ' for test ' + $1 + ': starting computation.'); }, t->threadId, t->op);

	for(int i = 0; i < 99999; ++i)
		for(int j = 0; j < N; ++j)
		{
			switch(t->op)
			{
				case 0: emscripten_atomic_add_u64(&sharedData[j], 1); break;
				case 1: emscripten_atomic_sub_u64(&sharedData[j], 1); break;
				case 2: emscripten_atomic_and_u64(&sharedData[j], ~(1UL << t->threadId)); break;
				case 3: emscripten_atomic_or_u64(&sharedData[j], 1UL << t->threadId); break;
				case 4: emscripten_atomic_xor_u64(&sharedData[j], 1UL << t->threadId); break;
				case 5:
				{
					// Atomically load and store data, and test that each individual u8 is the same.
					uint64_t data = emscripten_atomic_load_u64(&sharedData[j]);
					uint8_t dataU8[8];
					memcpy(dataU8, &data, 8);
					assert(dataU8[0] >= 10 && dataU8[0] < 10+NUM_THREADS);
					assert(dataU8[0] == dataU8[1] && dataU8[0] == dataU8[2] && dataU8[0] == dataU8[3]);
					assert(dataU8[0] == dataU8[4] && dataU8[0] == dataU8[5] && dataU8[0] == dataU8[6] && dataU8[0] == dataU8[7]);
					dataU8[0] = dataU8[1] = dataU8[2] = dataU8[3] = dataU8[4] = dataU8[5] = dataU8[6] = dataU8[7] = 10 + t->threadId;
					memcpy(&data, dataU8, 8);
					emscripten_atomic_store_u64(&sharedData[j], data);
				}
				break;
				case 6:
				{
					uint64_t newData = rand_60();
					uint64_t data;
					uint64_t prevData;
					do {
						data = emscripten_atomic_load_u64(&sharedData[j]);
						prevData = emscripten_atomic_cas_u64(&sharedData[j], data, newData);
					} while(prevData != data);
					threadCasAccumulatedReadData[t->threadId] += data;
					threadCasAccumulatedWrittenData[t->threadId] += newData;
				}
				break;
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

	switch(test)
	{
		case 2: memset(sharedData, 0xFF, sizeof(sharedData)); break;
		case 5: memset(sharedData, 0x10, sizeof(sharedData)); break;
		default: memset(sharedData, 0, sizeof(sharedData)); break;
	}

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
	if (test != 6)
	{
		for(int i = 1; i < N; ++i)
			assert(sharedData[i] == sharedData[0]);
	}
}

int main()
{
	globalDouble = 5.0;
	globalU64 = 5;

	int result = 0;
	if (!emscripten_has_threading_support())
	{
#ifdef REPORT_RESULT
		REPORT_RESULT();
#endif
		printf("Skipped: Threading is not supported.\n");
		return 0;
	}

	for(int i = 0; i < 7; ++i)
		RunTest(i);

	uint64_t totalRead = 0;
	uint64_t totalWritten = 0;
	for(int i = 0; i < NUM_THREADS; ++i)
	{
		totalRead += threadCasAccumulatedReadData[i];
		totalWritten += threadCasAccumulatedWrittenData[i];
	}
	for(int i = 0; i < N; ++i)
		totalRead += sharedData[i];
	if (totalRead == totalWritten)
		printf("totalRead: %llu, totalWritten: %llu\n", totalRead, totalWritten);
	else
		printf("64-bit CAS test failed! totalRead != totalWritten (%llu != %llu)\n", totalRead, totalWritten);
#ifdef REPORT_RESULT
	if (totalRead != totalWritten) result = 1;
	REPORT_RESULT();
#else
	EM_ASM(Module['print']('Main: Test successfully finished.'));
#endif
}
