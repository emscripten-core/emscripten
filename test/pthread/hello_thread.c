/*
 * Copyright 2017 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */

#include <pthread.h>
#include <emscripten.h>
#include <emscripten/console.h>

void *thread_main(void *arg)
{
	emscripten_out("hello from thread!");
	emscripten_force_exit(0);
	__builtin_trap();
}

int main()
{
	pthread_t thread;
	pthread_create(&thread, NULL, thread_main, NULL);
	emscripten_exit_with_live_runtime();
	__builtin_trap();
}
