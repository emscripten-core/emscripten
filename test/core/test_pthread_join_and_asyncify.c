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

// TODO Remove EMSCRIPTEN_KEEPALIVE when support for async attributes is enabled.
EMSCRIPTEN_KEEPALIVE void *run_thread(void *args) {
  int ret = async_call();
  assert(ret == 42);
  return NULL;
}

int main() {
  pthread_t id;
  pthread_create(&id, NULL, run_thread, NULL);
  printf("joining thread!\n");
  pthread_join(id, NULL);
  printf("joined thread!\n");

  return 0;
}
