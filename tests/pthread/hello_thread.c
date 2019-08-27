/*
 * Copyright 2017 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */

#include <pthread.h>
#include <emscripten.h>

void *thread_main(void *arg)
{
	EM_ASM(out('hello from thread!'));
#ifdef REPORT_RESULT
	REPORT_RESULT(1);
#endif
	return 0;
}

int main()
{
	pthread_t thread;
	pthread_create(&thread, NULL, thread_main, NULL);
	EM_ASM(noExitRuntime=true);
}
