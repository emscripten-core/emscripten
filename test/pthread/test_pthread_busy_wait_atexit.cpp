#include <stdatomic.h>
#include <stdbool.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <emscripten/console.h>

_Atomic bool done = false;

void exit_handler() {
  printf("exit_handler\n");
}

void* thread_main(void*) {
  // Avoid using printf here since stdio is proxied back to the
  // main thread which is busy looping
  emscripten_out("in thread");
  atexit(exit_handler);
  done = true;
  return NULL;
}

// Similar to test_pthread_busy_wait.cpp but with lower level pthreads
// API and explcit use of atexit before setting done to true.
// We also don't make any calls during the busy loop which means that
// proxied calls are *not* processed.
int main() {
  printf("in main\n");
  pthread_t t;
  pthread_create(&t, NULL, thread_main, NULL);

  while (!done) { }

  pthread_join(t, NULL);
  printf("done main\n");
  return 0;
}
