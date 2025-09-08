// Copyright 2025 The Emscripten Authors.  All rights reserved.
// Emscripten is available under two separate licenses, the MIT license and the
// University of Illinois/NCSA Open Source License.  Both these licenses can be
// found in the LICENSE file.

#include <pthread.h>
#include <emscripten/console.h>

void *pthread_main(void *arg) {
  emscripten_out("thread has started.");
  return 0;
}

int main() {
  pthread_t t;
  pthread_create(&t, 0, pthread_main, 0);
  pthread_join(t, 0);
  emscripten_out("thread has quit.");
}
