// Copyright 2015 The Emscripten Authors.  All rights reserved.
// Emscripten is available under two separate licenses, the MIT license and the
// University of Illinois/NCSA Open Source License.  Both these licenses can be
// found in the LICENSE file.

#include <assert.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

void destructor(void* arg) {
#if defined(RETURN) || defined(EXIT)
  // The destructors for thread-specific data should only be executed
  // when a thread exits or when pthread_exit is explicitly called.
  assert(0 && "pthread key dtor should not be executed on application exit");
#else
  printf("destructor: %ld\n", (long)arg);
  assert(arg == (void*)42);
#endif
}

int main() {
  pthread_key_t key;
  pthread_key_create(&key, destructor);
  void *val = pthread_getspecific(key);
  assert(val == 0);
  pthread_setspecific(key, (void*)42);
  val = pthread_getspecific(key);
  assert(val == (void*)42);
  printf("done!\n");
#ifdef RETURN
  return 0;
#elif defined(EXIT)
  exit(0);
#else
  pthread_exit(0);
#endif
}
