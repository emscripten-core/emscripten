// Copyright 2016 The Emscripten Authors.  All rights reserved.
// Emscripten is available under two separate licenses, the MIT license and the
// University of Illinois/NCSA Open Source License.  Both these licenses can be
// found in the LICENSE file.

#include <emscripten/threading.h>
#include <stdio.h>
#include <assert.h>
#include <math.h>

_Atomic int func_called = 0;

void v() {
  func_called++;
}

void test_sync() {
  printf("Testing sync proxied runs:\n");
  func_called = 0;
  for (int i = 0; i < 1000; ++i) {
    emscripten_sync_run_in_main_runtime_thread(EM_FUNC_SIG_V, v);
    assert(func_called == i+1);
  }
}

void test_async() {
  printf("Testing async proxied runs:\n");
  func_called = 0;
  for (int i = 0; i < 1000; ++i) {
    emscripten_async_run_in_main_runtime_thread(EM_FUNC_SIG_V, v);
  }

  while (func_called != 1000) {}
}

void test_async_waitable() {
  printf("Testing waitable async proxied runs:\n");
  func_called = 0;
  em_queued_call* handles[1000];

  for (int i = 0; i < 1000; ++i) {
    handles[i] = emscripten_async_waitable_run_in_main_runtime_thread(EM_FUNC_SIG_V, v);
  }

  EMSCRIPTEN_RESULT r = emscripten_wait_for_call_v(handles[999], INFINITY);
  assert(r == EMSCRIPTEN_RESULT_SUCCESS);

  // Since ordering is guaranteed we know that all the other tasks must
  // also have already been completed.
  int final_count = func_called;
  printf("final_count: %d\n", final_count);
  assert(final_count == 1000);

  // Cleanup/free all the handles.  Waiting on the 1000th a second time is
  // allowed by the API.
  for (int i = 0; i < 1000; ++i) {
    EMSCRIPTEN_RESULT r = emscripten_wait_for_call_v(handles[i], INFINITY);
    assert(r == EMSCRIPTEN_RESULT_SUCCESS);
    emscripten_async_waitable_close(handles[i]);
  }
}

void *thread_main(void* arg) {
  test_sync();
  test_async();
  test_async_waitable();
  printf("thread_main done\n");
  pthread_exit(NULL);
}

int main() {
  if (emscripten_has_threading_support()) {
    test_sync();
    test_async_waitable();

    pthread_t thread;
    int rc = pthread_create(&thread, NULL, thread_main, NULL);
    assert(rc == 0);
    void* retval;
    rc = pthread_join(thread, &retval);
    assert(rc == 0);
    printf("pthread_join done: %ld\n", (intptr_t)retval);
    assert(retval == NULL);
  }

  test_async();

  return 0;
}
