/*
 * Copyright 2017 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */

#include <assert.h>
#include <stdbool.h>
#include <stdio.h>

#include <emscripten/em_asm.h>
#include <emscripten/eventloop.h>
#include <emscripten/threading.h>

_Atomic bool done_callback = false;

void main_thread_callback(void* user_data) {
  printf("main_thread_callback called\n");
  done_callback = true;
}

void set_timeout_on_main() {
  int is_worker = EM_ASM_INT(return ENVIRONMENT_IS_WORKER);
  printf("main: ENVIRONMENT_IS_WORKER: %d\n", is_worker);
  // Verify that we can do async work here on the main thread
  // without causing the runtime to exit.
  emscripten_set_immediate(main_thread_callback, NULL);
}

int main() {
  int is_worker = EM_ASM_INT(return ENVIRONMENT_IS_WORKER);
  printf("ENVIRONMENT_IS_WORKER: %d\n", is_worker);
  assert(is_worker);

  emscripten_sync_run_in_main_runtime_thread(EM_FUNC_SIG_V, set_timeout_on_main);
  while (!done_callback) {}

  // Repeat the same process to ensure the main thread is still responsive.
  done_callback = false;

  emscripten_sync_run_in_main_runtime_thread(EM_FUNC_SIG_V, set_timeout_on_main);
  while (!done_callback) {}

  printf("main done\n");
  return 0;
}
