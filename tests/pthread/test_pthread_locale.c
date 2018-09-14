/*
 * Copyright 2017 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
// #include <emscripten/emscripten.h>
#include <emscripten/threading.h>
#include "../../system/lib/libc/musl/src/internal/pthread_impl.h"

#define NUM_THREADS  1

locale_t do_test() {
  pthread_t thread = pthread_self();
  locale_t loc = thread->locale;
  printf("  pthread_self() = %p\n", thread);
  printf("  pthread_self()->locale = %p\n", loc);

  if (!loc) {
    puts("ERROR: loc is null");
  }
  return loc;
}

void *thread_test(void *t) 
{
  puts("Doing test in child thread");
  pthread_exit((void*)do_test());
}

int main (int argc, char *argv[])
{
  puts("Doing test in main thread");
  locale_t main_loc = do_test();
  locale_t child_loc;

  if (!emscripten_has_threading_support())
  {
    child_loc = main_loc;
  }
  else
  {
    long id = 1;
    pthread_t thread;

    pthread_create(&thread, NULL, thread_test, (void *)id);

    pthread_join(thread, (void**)&child_loc);
  }

#ifdef REPORT_RESULT
  int result = (main_loc == child_loc);
  REPORT_RESULT(result);
#endif

  pthread_exit(NULL);
}
