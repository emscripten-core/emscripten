#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
// #include <emscripten/emscripten.h>
#include <emscripten/threading.h>
#include "../../system/lib/libc/musl/src/internal/pthread_impl.h"

#define NUM_THREADS  1

void do_test() {
  pthread_t thread = pthread_self();
  locale_t loc = thread->locale;
  printf("  pthread_self() = %p\n", thread);
  printf("  pthread_self()->locale = %p\n", loc);

  if (!loc) {
    puts("ERROR: loc is null");
    abort();
  }
}

void *thread_test(void *t) 
{
  puts("Doing test in child thread");
  do_test();
  pthread_exit(NULL);
}

int main (int argc, char *argv[])
{
  puts("Doing test in main thread");
  do_test();

  if (emscripten_has_threading_support())
  {
    long id = 1;
    pthread_t threads[NUM_THREADS];
    pthread_attr_t attr;

    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);
    pthread_create(&threads[0], &attr, thread_test, (void *)id);

    /* Wait for all threads to complete */
    for (int i=0; i<NUM_THREADS; i++) {
      pthread_join(threads[i], NULL);
    }
    printf ("Main(): Waited on %d  threads. Done.\n", NUM_THREADS);
  }

#ifdef REPORT_RESULT
  int result = 0;
  REPORT_RESULT();
#endif

  pthread_exit(NULL);
}
