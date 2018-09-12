// Copyright 2015 The Emscripten Authors.  All rights reserved.
// Emscripten is available under two separate licenses, the MIT license and the
// University of Illinois/NCSA Open Source License.  Both these licenses can be
// found in the LICENSE file.

#include <assert.h>
#include <stdio.h>
#include <pthread.h>
#include <emscripten.h>
#include <emscripten/threading.h>

volatile int numInitialized = 0;

void once_init()
{
	emscripten_atomic_add_u32((void*)&numInitialized, 1);
}

#define NUM_THREADS 8

void *thread_main(void *arg)
{
	static pthread_once_t control = PTHREAD_ONCE_INIT;
	pthread_once(&control, &once_init);
	assert(numInitialized == 1);
	pthread_exit(0);
}

pthread_t thread[NUM_THREADS];

int main()
{
	assert(numInitialized == 0);
	for(int i = 0; i < NUM_THREADS; ++i) pthread_create(&thread[i], NULL, thread_main, 0);

	if (emscripten_has_threading_support()) {
		for(int i = 0; i < NUM_THREADS; ++i) pthread_join(thread[i], NULL);
		assert(numInitialized == 1);
	}

#ifdef REPORT_RESULT
	REPORT_RESULT(0);
#endif
}
