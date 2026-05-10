// Copyright 2023 The Emscripten Authors.  All rights reserved.
// Emscripten is available under two separate licenses, the MIT license and the
// University of Illinois/NCSA Open Source License.  Both these licenses can be
// found in the LICENSE file.

#include <emscripten.h>
#include <pthread.h>
#include <stdio.h>
#include <assert.h>

EM_ASYNC_JS(int, async_call, (), {
  await new Promise((resolve) => setTimeout(resolve, 0));
  return 42;
});

void *run_thread(void *args) {
  intptr_t ret = async_call();
  printf("async_call done %ld\n", ret);
  assert(ret == 42);
  return (void*)ret;
}

int main() {
  pthread_t id;
  // Test that JSPI works on the main thread.
  emscripten_sleep(1);
  // Also test that JSPI works on other threads.
  pthread_create(&id, NULL, run_thread, NULL);
  printf("joining thread!\n");
  void* rtn;
  pthread_join(id, &rtn);
  printf("join returned -> %ld\n", (intptr_t)rtn);
  assert((intptr_t)rtn == 42);
  return 0;
}
