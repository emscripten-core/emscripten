#include <assert.h>
#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>

pthread_t t;

void* thread_main_exit(void* arg) {
  printf("calling exit\n");
  exit(42);
}

int main() {
  printf("main\n");
  int rc = pthread_create(&t, NULL, thread_main_exit, NULL);
  assert(rc == 0);
  void* thread_rtn = 0;
  rc = pthread_join(t, &thread_rtn);
  assert(rc == 0);
#if EXIT_RUNTIME
  printf("done join -- should never get here\n");
  return 1;
#else
  // Since EXIT_RUNTIME is not set the exit() in the thread is not expected to
  // bring down the whole process, only itself.
  printf("done join -- thread exited with %ld\n", (intptr_t)thread_rtn);
#ifdef REPORT_RESULT
  REPORT_RESULT(43);
#endif
  return 43;
#endif
}
