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
  pthread_create(&t, NULL, thread_main_exit, NULL);
  pthread_join(t, NULL);
  printf("done join\n");
#ifdef REPORT_RESULT
  REPORT_RESULT(0);
#endif
  return 0;
}
