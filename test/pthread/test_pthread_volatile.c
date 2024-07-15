// Copyright 2015 The Emscripten Authors.  All rights reserved.
// Emscripten is available under two separate licenses, the MIT license and the
// University of Illinois/NCSA Open Source License.  Both these licenses can be
// found in the LICENSE file.

#include <assert.h>
#include <pthread.h>
#include <stdio.h>
#include <errno.h>

// Toggle to use two different methods for updating shared data (C++03 volatile
// vs explicit atomic ops).  Note that using a volatile variable explicitly
// depends on x86 strong memory model semantics.
//#define USE_C_VOLATILE

#ifdef USE_C_VOLATILE
volatile
#else
_Atomic
#endif
int sharedVar = 0;

static void *thread_start(void *arg) { // thread: just flip the shared flag and quit.
  sharedVar = 1;
  pthread_exit(0);
}

int main() {
  pthread_t thr;
  int rc = pthread_create(&thr, NULL, thread_start, (void*)0);
  if (rc != 0) {
    return 1;
  }

  while(sharedVar == 0) {}

  assert(sharedVar == 1);
  return 0;
}
