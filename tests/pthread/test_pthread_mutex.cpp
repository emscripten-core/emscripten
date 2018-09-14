// Copyright 2015 The Emscripten Authors.  All rights reserved.
// Emscripten is available under two separate licenses, the MIT license and the
// University of Illinois/NCSA Open Source License.  Both these licenses can be
// found in the LICENSE file.

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <emscripten.h>
#include <emscripten/threading.h>
#include <unistd.h>

#define NUM_THREADS 8

int numThreadsToCreateTotal = 50;

pthread_t thread[NUM_THREADS] = {};

volatile int counter = 0; // Shared data
pthread_mutex_t lock;

void sleep(int msecs)
{
	// Test two different variants of sleeping to verify
	// against bug https://bugzilla.mozilla.org/show_bug.cgi?id=1131757
#ifdef SPINLOCK_TEST
	double t0 = emscripten_get_now();
	double t1 = t0 + (double)msecs;
	while(emscripten_get_now() < t1)
		;
#else
	usleep(msecs*1000);
#endif
}
void *ThreadMain(void *arg)
{
	pthread_mutex_lock(&lock);
	int c = counter;
	sleep(100); // Create contention on the lock.
	++c;
	counter = c;
	pthread_mutex_unlock(&lock);
	pthread_exit(0);
}

void CreateThread(int i, int n)
{
	pthread_attr_t attr;
	pthread_attr_init(&attr);
	pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);
	pthread_attr_setstacksize(&attr, 4*1024);
	int rc = pthread_create(&thread[i], &attr, ThreadMain, 0);
	if (rc != 0 || thread[i] == 0)
		printf("Failed to create thread!\n");
	pthread_attr_destroy(&attr);
}

int threadNum = 0;
void WaitToJoin()
{
	int threadsRunning = 0;
	// Join all threads.
	for(int i = 0; i < NUM_THREADS; ++i)
	{
		if (thread[i])
		{
			void *status;
			int rc = pthread_join(thread[i], &status);
			if (rc == 0)
			{
				thread[i] = 0;
				if (threadNum < numThreadsToCreateTotal)
				{
					CreateThread(i, threadNum++);
					++threadsRunning;
				}
			}
			else
				++threadsRunning;
		}
	}
	if (!threadsRunning)
	{
		if (counter == numThreadsToCreateTotal)
			EM_ASM(console.log('All threads finished. Counter = ' + $0 + ' as expected.'), counter);
		else
			EM_ASM(console.error('All threads finished, but counter = ' + $0 + ' != ' + $1 + '!'), counter, numThreadsToCreateTotal);
#ifdef REPORT_RESULT
		REPORT_RESULT(counter);
#endif
		emscripten_cancel_main_loop();
	}
}

int main()
{
	pthread_mutexattr_t attr;
	pthread_mutexattr_init(&attr);
	pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE);
	pthread_mutex_init(&lock, &attr);

	pthread_mutex_lock(&lock);
	pthread_mutex_unlock(&lock);

	if (emscripten_has_threading_support()) {
		// Create new threads in parallel.
		for(int i = 0; i < NUM_THREADS; ++i)
			CreateThread(i, threadNum++);

		emscripten_set_main_loop(WaitToJoin, 0, 0);
	} else {
#ifdef REPORT_RESULT
		REPORT_RESULT(50);
#endif
	}
}
