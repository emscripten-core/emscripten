// Copyright 2016 The Emscripten Authors.  All rights reserved.
// Emscripten is available under two separate licenses, the MIT license and the
// University of Illinois/NCSA Open Source License.  Both these licenses can be
// found in the LICENSE file.

#include <emscripten/threading.h>
#include <stdio.h>
#include <assert.h>
#include <math.h>

volatile int func_called = 0;

void v()
{
	emscripten_atomic_add_u32((void*)&func_called, 1);
}

void test_sync()
{
	printf("Testing sync proxied runs:\n");
	emscripten_atomic_store_u32((void*)&func_called, 0);
	for(int i = 0; i < 1000; ++i)
	{
		emscripten_sync_run_in_main_runtime_thread(EM_FUNC_SIG_V, v);
		assert(emscripten_atomic_load_u32((void*)&func_called) == i+1);
	}
}

void test_async()
{
	printf("Testing async proxied runs:\n");
	emscripten_atomic_store_u32((void*)&func_called, 0);
	for(int i = 0; i < 1000; ++i)
	{
		emscripten_async_run_in_main_runtime_thread(EM_FUNC_SIG_V, v);
	}

	while(emscripten_atomic_load_u32((void*)&func_called) != 1000)
		;
}

void test_async_waitable()
{
	printf("Testing waitable async proxied runs:\n");
	emscripten_atomic_store_u32((void*)&func_called, 0);
	for(int i = 0; i < 1000; ++i)
	{
		em_queued_call *c = emscripten_async_waitable_run_in_main_runtime_thread(EM_FUNC_SIG_V, v);
		if (i == 999)
		{
			EMSCRIPTEN_RESULT r = emscripten_wait_for_call_v(c, INFINITY);
			assert(r == EMSCRIPTEN_RESULT_SUCCESS);
		}
		emscripten_async_waitable_close(c);
	}
	assert(func_called == 1000);
}

void *thread_main(void*)
{
	test_sync();
	test_async();
	test_async_waitable();
	pthread_exit(0);
}

int main()
{
	if (emscripten_has_threading_support())
	{
		test_sync();
		test_async_waitable();

		pthread_attr_t attr;
		pthread_attr_init(&attr);
		pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);
		pthread_t thread;
		int rc = pthread_create(&thread, &attr, thread_main, 0);
		assert(rc == 0);
		rc = pthread_join(thread, 0);
		assert(rc == 0);
	}

	test_async();

#ifdef REPORT_RESULT
	REPORT_RESULT(0);
#endif
}
