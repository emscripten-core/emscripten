// Copyright 2019 The Emscripten Authors.  All rights reserved.
// Emscripten is available under two separate licenses, the MIT license and the
// University of Illinois/NCSA Open Source License.  Both these licenses can be
// found in the LICENSE file.

#include <assert.h>
#include <emscripten.h>
#include <pthread.h>
#include <stdio.h>

int main() {
  if (!emscripten_has_threading_support())
  {
#ifdef REPORT_RESULT
    REPORT_RESULT(0);
#endif
    printf("Skipped: Threading is not supported.\n");
    return 0;
  }

  // This should fail on the main thread.
  puts("trying to block...");
  pthread_cond_t cv = PTHREAD_COND_INITIALIZER;
  pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
  pthread_cond_wait(&cv, &lock);
  puts("blocked ok.");
#ifdef REPORT_RESULT
  REPORT_RESULT(1);
#endif
}

