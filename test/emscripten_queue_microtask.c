/*
 * Copyright 2025 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */

// Test emscripten_queue_microtask() behavior

#include <stdio.h>
#include <assert.h>
#include <emscripten.h>
#include <emscripten/html5.h>

void cb(void *userData) {
  printf("cb\n");
  assert(userData == (void*)42);
  emscripten_force_exit(0);
}

int main() {
  emscripten_queue_microtask(cb, (void*)42);
  emscripten_exit_with_live_runtime();
  return 99; // We won't reach here, but return non-zero value to guard against refactors that might exit() with this value.
}
