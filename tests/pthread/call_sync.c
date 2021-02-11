/*
 * Copyright 2020 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */

#include <stdio.h>
#include <assert.h>
#include <stdbool.h>
#include <pthread.h>
#include <emscripten.h>
#include <emscripten/threading.h>

static _Atomic bool started = false;

void *new_thread(void* ctx) {
  started = true;
  emscripten_exit_with_live_runtime();
  return NULL;
}

int magic_number() {
  assert(emscripten_main_browser_thread_id() != pthread_self());
  return 42;
}

int main() {
  pthread_t worker;
  assert(pthread_create(&worker, NULL, new_thread, NULL) == 0);
  while (!started)
    emscripten_sleep(100);

  assert(emscripten_dispatch_to_thread_sync(worker, EM_FUNC_SIG_I, &magic_number, NULL) == 42);

#ifdef REPORT_RESULT
  REPORT_RESULT(1);
#endif
}
