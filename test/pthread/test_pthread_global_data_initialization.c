/*
 * Copyright 2017 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */

#include <assert.h>
#include <pthread.h>
#include <emscripten/console.h>

int globalData = 1;

void *thread_main(void *arg)
{
  emscripten_outf("hello from pthread 1: %d", globalData);
  assert(globalData == 10);

  globalData = 20;
  emscripten_outf("hello from pthread 2: %d", globalData);
  assert(globalData == 20);
  return 0;
}

int main()
{
  emscripten_outf("hello from main 1: %d", globalData);
  assert(globalData == 1);

  globalData = 10;
  emscripten_outf("hello from main 2: %d", globalData);
  assert(globalData == 10);

  pthread_t thread;
  pthread_create(&thread, NULL, thread_main, NULL);
  pthread_join(thread, 0);

  emscripten_outf("hello from main 3: %d", globalData);
  assert(globalData == 20);
  return 0;
}
