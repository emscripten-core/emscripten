#include <assert.h>
#include <pthread.h>
#include <stdbool.h>
#include <stdio.h>

pthread_mutex_t m = PTHREAD_MUTEX_INITIALIZER;

int main() {
  printf("in main\n");
  int rtn = pthread_mutex_lock(&m);
  assert(rtn == 0);

  // Attempt to lock a second time.  In debug builds this should
  // hit an assertion.  In release builds this will deadlock and
  // never return.
  pthread_mutex_lock(&m);
  printf("should never get here\n");
  assert(false);
  return 0;
}
