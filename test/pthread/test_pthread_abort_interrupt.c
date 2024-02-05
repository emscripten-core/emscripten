// Copyright 2022 The Emscripten Authors.  All rights reserved.
// Emscripten is available under two separate licenses, the MIT license and the
// University of Illinois/NCSA Open Source License.  Both these licenses can be
// found in the LICENSE file.

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

// Test that when a thread aborts the whole application crashes
// even when the main thread is blocking in pthread_join and
// not returning the event loop.

void* thread_start(void* arg) {
  puts("thread_start");
  abort();
  // Should never get here
  __builtin_trap();
  return NULL;
}

int main() {
  puts("in main");
  pthread_t t;
  pthread_create(&t, NULL, thread_start, NULL);
  puts("calling join");
  pthread_join(t, NULL);
  // Should never get here
  __builtin_trap();
}

