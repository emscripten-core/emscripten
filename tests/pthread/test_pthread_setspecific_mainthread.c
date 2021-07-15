// Copyright 2015 The Emscripten Authors.  All rights reserved.
// Emscripten is available under two separate licenses, the MIT license and the
// University of Illinois/NCSA Open Source License.  Both these licenses can be
// found in the LICENSE file.

#include <assert.h>
#include <stdio.h>
#include <pthread.h>

void destructor(void* arg) {
  printf("destructor: %ld\n", (long)arg);
  assert(arg == (void*)42);
}

int main() {
  pthread_key_t key;
  pthread_key_create(&key, destructor);
  void *val = pthread_getspecific(key);
  assert(val == 0);
  pthread_setspecific(key, (void*)42);
  val = pthread_getspecific(key);
  assert(val == (void*)42);
  return 0;
}
