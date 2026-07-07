// Copyright 2026 The Emscripten Authors.  All rights reserved.
// Emscripten is available under two separate licenses, the MIT license and the
// University of Illinois/NCSA Open Source License.  Both these licenses can be
// found in the LICENSE file.

#include <assert.h>
#include <pthread.h>
#include <stdio.h>
#include <emscripten/stack.h>

void* thread_main_overflow(void* arg) {
  uintptr_t stack_end = emscripten_stack_get_end();
  // Overwrite 500 bytes into the guard region below stack_end and the 8-byte stack cookie at stack_end.
  // Total 508 bytes (well under default guard_size = 8192).
  volatile char* ptr = (volatile char*)(stack_end - 500);
  for (int i = 0; i < 508; i++) {
    ptr[i] = 0xAA;
  }
  printf("thread_main_overflow done\n");
  return NULL;
}

int main() {
  pthread_t t;
  assert(pthread_create(&t, NULL, thread_main_overflow, NULL) == 0);
  assert(pthread_join(t, NULL) == 0);
  printf("main done\n");
  return 0;
}
