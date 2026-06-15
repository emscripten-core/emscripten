// Copyright 2019 The Emscripten Authors.  All rights reserved.
// Emscripten is available under two separate licenses, the MIT license and the
// University of Illinois/NCSA Open Source License.  Both these licenses can be
// found in the LICENSE file.

#include <assert.h>
#include <pthread.h>
#include <stdbool.h>

// tests, in the following sequence
//  pthread_key_create with key k
//  pthread_key_delete of key k
//  pthread_key_create returning the same key k
// , that the thread local slot is correctly initialized to 0 in all threads

// overall structure based on test_pthread_reltime.cpp

static pthread_key_t key;

static pthread_barrier_t sync_barrier;

void *thread_main(void *arg) {
  assert(pthread_getspecific(key) == 0);

  pthread_setspecific(key, (void*)123);
  assert(pthread_getspecific(key) == (void*)123);

  // 1. Wait for slot to be recreated. First signal that we are done setting our value.
  pthread_barrier_wait(&sync_barrier);

  // 2. Wait for main thread to finish recreating the key.
  pthread_barrier_wait(&sync_barrier);

  assert(pthread_getspecific(key) == 0);

  return NULL;
}

int main() {
  // create tls slot and set the value
  pthread_key_create(&key, NULL);
  assert(pthread_getspecific(key) == 0);

  pthread_setspecific(key, (void*)456);
  assert(pthread_getspecific(key) == (void*)456);

  pthread_barrier_init(&sync_barrier, NULL, 2);

  // launch worker thread that also sets and queries the same slot
  pthread_attr_t attr;
  pthread_attr_init(&attr);

  pthread_t thread;
  int error = pthread_create(&thread, &attr, thread_main, NULL);
  assert(!error);

  // 1. Wait for worker thread to finish initial setting of the slot
  pthread_barrier_wait(&sync_barrier);

  // recreate
  pthread_key_delete(key);
  pthread_key_t oldkey = key;

  pthread_key_create(&key, NULL);
  assert(pthread_getspecific(key) == 0);

  // this assertion holds in the current musl implementation,
  // so we can skip having to loop over pthread_key_create's
  // until we recover the same key
  assert(oldkey == key);

  // 2. Notify worker thread to check the slot again
  pthread_barrier_wait(&sync_barrier);

  pthread_join(thread, NULL);

  return 0;
}
