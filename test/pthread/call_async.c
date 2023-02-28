/*
 * Copyright 2020 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */

#include <stdio.h>
#include <assert.h>
#include <emscripten.h>
#include <emscripten/threading.h>

int state = 0;

void increment() {
  state++;
}

void finish() {
  assert(state == 2);
  emscripten_force_exit(0);
}

int main() {
  assert(state == 0);
  // This dispatch_to_thread call will be synchronous since we are on the right
  // thread already.
  int called_now = emscripten_dispatch_to_thread(emscripten_main_runtime_thread_id(), EM_FUNC_SIG_V, &increment, 0);
  assert(called_now);
  assert(state == 1);
  // This async call will actually be async.
  emscripten_dispatch_to_thread_async(emscripten_main_runtime_thread_id(), EM_FUNC_SIG_V, &increment, 0);
  assert(state == 1);
  emscripten_dispatch_to_thread_async(emscripten_main_runtime_thread_id(), EM_FUNC_SIG_V, &finish, 0);
  emscripten_exit_with_live_runtime();
  __builtin_trap();
}
