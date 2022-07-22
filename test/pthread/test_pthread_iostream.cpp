// Copyright 2015 The Emscripten Authors.  All rights reserved.
// Emscripten is available under two separate licenses, the MIT license and the
// University of Illinois/NCSA Open Source License.  Both these licenses can be
// found in the LICENSE file.

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <assert.h>
#include <iostream>

void *ThreadMain(void *arg)
{
	std::cout << "Hello from thread" << std::endl;

	return 0;
}

int numThreadsToCreate = 1000;

int main()
{
	pthread_t thread;
	int rc = pthread_create(&thread, NULL, ThreadMain, 0);
	assert(rc == 0);

	rc = pthread_join(thread, NULL);
	assert(rc == 0);

	std::cout << "The thread should print 'Hello from thread'" << std::endl;
	return 0;
}
