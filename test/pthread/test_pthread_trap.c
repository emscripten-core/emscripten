#include <assert.h>
#include <pthread.h>
#include <stdio.h>

void* thread_main() {
  __builtin_trap();
}

int main() {
  printf("in main\n");

  pthread_t t;
  int rc = pthread_create(&t, NULL, thread_main, NULL);
  assert(rc == 0);

  pthread_join(t, NULL);
  printf("should never get here\n");
  return 99;
}
