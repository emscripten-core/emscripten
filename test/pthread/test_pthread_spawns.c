// Copyright 2015 The Emscripten Authors.  All rights reserved.
// Emscripten is available under two separate licenses, the MIT license and the
// University of Illinois/NCSA Open Source License.  Both these licenses can be
// found in the LICENSE file.

#include <pthread.h>
#include <emscripten/threading.h>

#define NUM_THREADS 2

void *thread_main(void *arg)
{
	pthread_exit(0);
}

pthread_t thread[NUM_THREADS];

int main()
{
	for(int x = 0; x < 100; ++x)
	{
		for(int i = 0; i < NUM_THREADS; ++i) pthread_create(&thread[i], NULL, thread_main, 0);
		if (emscripten_has_threading_support())
			for(int i = 0; i < NUM_THREADS; ++i) pthread_join(thread[i], NULL);
	}
	return 0;
}
