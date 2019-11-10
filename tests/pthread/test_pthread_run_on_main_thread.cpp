// Copyright 2016 The Emscripten Authors.  All rights reserved.
// Emscripten is available under two separate licenses, the MIT license and the
// University of Illinois/NCSA Open Source License.  Both these licenses can be
// found in the LICENSE file.

#include <emscripten/threading.h>
#include <stdio.h>
#include <assert.h>
#include <math.h>

int v_called = 0;
void v()
{
	assert(emscripten_is_main_runtime_thread());
	printf("Hello!\n");
	v_called = 1;
}

int vi_called = 0;
void vi(int param0)
{
	assert(emscripten_is_main_runtime_thread());
	printf("Hello %d!\n", param0);
	vi_called = 1;
}

int vii_called = 0;
void vii(int param0, int param1)
{
	assert(emscripten_is_main_runtime_thread());
	printf("Hello %d %d!\n", param0, param1);
	vii_called = 1;
}

int viii_called = 0;
void viii(int param0, int param1, int param2)
{
	assert(emscripten_is_main_runtime_thread());
	printf("Hello %d %d %d!\n", param0, param1, param2);
	viii_called = 1;
}

int i_called = 0;
int i()
{
	assert(emscripten_is_main_runtime_thread());
	printf("Hello i!\n");
	i_called = 1;
	return 84;
}

int ii_called = 0;
int ii(int param0)
{
	assert(emscripten_is_main_runtime_thread());
	printf("Hello ii %d!\n", param0);
	ii_called = 1;
	return 85;
}

int iii_called = 0;
int iii(int param0, int param1)
{
	assert(emscripten_is_main_runtime_thread());
	printf("Hello iii %d %d!\n", param0, param1);
	iii_called = 1;
	return 86;
}

int iiii_called = 0;
int iiii(int param0, int param1, int param2)
{
	assert(emscripten_is_main_runtime_thread());
	printf("Hello iiii %d %d %d!\n", param0, param1, param2);
	iiii_called = 1;
	return 87;
}

void test_sync()
{
	printf("Testing sync proxied runs:\n");
	int ret;
	v_called = 0; emscripten_sync_run_in_main_runtime_thread(EM_FUNC_SIG_V, v); assert(v_called == 1);
	vi_called = 0; emscripten_sync_run_in_main_runtime_thread(EM_FUNC_SIG_VI, vi, 42); assert(vi_called == 1);
	vii_called = 0; emscripten_sync_run_in_main_runtime_thread(EM_FUNC_SIG_VII, vii, 42, 43); assert(vii_called == 1);
	viii_called = 0; emscripten_sync_run_in_main_runtime_thread(EM_FUNC_SIG_VIII, viii, 42, 43, 44); assert(viii_called == 1);
	i_called = 0; ret = emscripten_sync_run_in_main_runtime_thread(EM_FUNC_SIG_I, i); assert(i_called == 1); assert(ret == 84);
	ii_called = 0; ret = emscripten_sync_run_in_main_runtime_thread(EM_FUNC_SIG_II, ii, 42); assert(ii_called == 1); assert(ret == 85);
	iii_called = 0; ret = emscripten_sync_run_in_main_runtime_thread(EM_FUNC_SIG_III, iii, 42, 43); assert(iii_called == 1); assert(ret == 86);
	iiii_called = 0; ret = emscripten_sync_run_in_main_runtime_thread(EM_FUNC_SIG_IIII, iiii, 42, 43, 44); assert(iiii_called == 1); assert(ret == 87);
}

void test_async()
{
	printf("Testing async proxied runs:\n");
	emscripten_async_run_in_main_runtime_thread(EM_FUNC_SIG_V, v);
	emscripten_async_run_in_main_runtime_thread(EM_FUNC_SIG_VI, vi, 42);
	emscripten_async_run_in_main_runtime_thread(EM_FUNC_SIG_VII, vii, 42, 43);
	emscripten_async_run_in_main_runtime_thread(EM_FUNC_SIG_VIII, viii, 42, 43, 44);
	emscripten_async_run_in_main_runtime_thread(EM_FUNC_SIG_I, i);
	emscripten_async_run_in_main_runtime_thread(EM_FUNC_SIG_II, ii, 42);
	emscripten_async_run_in_main_runtime_thread(EM_FUNC_SIG_III, iii, 42, 43);
	emscripten_async_run_in_main_runtime_thread(EM_FUNC_SIG_IIII, iiii, 42, 43, 44);
}

void test_async_waitable()
{
	printf("Testing waitable async proxied runs:\n");
	em_queued_call *c1 = emscripten_async_waitable_run_in_main_runtime_thread(EM_FUNC_SIG_V, v);
	em_queued_call *c2 = emscripten_async_waitable_run_in_main_runtime_thread(EM_FUNC_SIG_VI, vi, 42);
	em_queued_call *c3 = emscripten_async_waitable_run_in_main_runtime_thread(EM_FUNC_SIG_VII, vii, 42, 43);
	em_queued_call *c4 = emscripten_async_waitable_run_in_main_runtime_thread(EM_FUNC_SIG_VIII, viii, 42, 43, 44);
	em_queued_call *c5 = emscripten_async_waitable_run_in_main_runtime_thread(EM_FUNC_SIG_I, i);
	em_queued_call *c6 = emscripten_async_waitable_run_in_main_runtime_thread(EM_FUNC_SIG_II, ii, 42);
	em_queued_call *c7 = emscripten_async_waitable_run_in_main_runtime_thread(EM_FUNC_SIG_III, iii, 42, 43);
	em_queued_call *c8 = emscripten_async_waitable_run_in_main_runtime_thread(EM_FUNC_SIG_IIII, iiii, 42, 43, 44);
	EMSCRIPTEN_RESULT r = emscripten_wait_for_call_v(c1, INFINITY); assert(r == EMSCRIPTEN_RESULT_SUCCESS);
	r = emscripten_wait_for_call_v(c2, INFINITY); assert(r == EMSCRIPTEN_RESULT_SUCCESS);
	r = emscripten_wait_for_call_v(c3, INFINITY); assert(r == EMSCRIPTEN_RESULT_SUCCESS);
	r = emscripten_wait_for_call_v(c4, INFINITY); assert(r == EMSCRIPTEN_RESULT_SUCCESS);
	int r1; r = emscripten_wait_for_call_i(c5, INFINITY, &r1); assert(r1 == 84); assert(r == EMSCRIPTEN_RESULT_SUCCESS);
	int r2; r = emscripten_wait_for_call_i(c6, INFINITY, &r2); assert(r2 == 85); assert(r == EMSCRIPTEN_RESULT_SUCCESS);
	int r3; r = emscripten_wait_for_call_i(c7, INFINITY, &r3); assert(r3 == 86); assert(r == EMSCRIPTEN_RESULT_SUCCESS);
	int r4; r = emscripten_wait_for_call_i(c8, INFINITY, &r4); assert(r4 == 87); assert(r == EMSCRIPTEN_RESULT_SUCCESS);
	emscripten_async_waitable_close(c1);
	emscripten_async_waitable_close(c2);
	emscripten_async_waitable_close(c3);
	emscripten_async_waitable_close(c4);
	emscripten_async_waitable_close(c5);
	emscripten_async_waitable_close(c6);
	emscripten_async_waitable_close(c7);
	emscripten_async_waitable_close(c8);
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
