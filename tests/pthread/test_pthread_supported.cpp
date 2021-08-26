// Copyright 2015 The Emscripten Authors.  All rights reserved.
// Emscripten is available under two separate licenses, the MIT license and the
// University of Illinois/NCSA Open Source License.  Both these licenses can be
// found in the LICENSE file.

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <emscripten/emscripten.h>
#include <emscripten/threading.h>
#include <assert.h>
#include <errno.h>

void *ThreadMain(void *arg)
{
	pthread_exit(NULL);
}

int main()
{
	pthread_t thread;
	int rc = pthread_create(&thread, NULL, ThreadMain, NULL);
	pthread_join(thread, NULL);

	if (emscripten_has_threading_support()) assert(rc == 0);
	else assert(rc == EAGAIN);

	return 0;
}
