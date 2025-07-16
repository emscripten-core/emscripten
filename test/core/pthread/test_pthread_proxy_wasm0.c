#include <stdio.h>
#include <pthread.h>

void *thread_main(void *arg) {
  printf("Hello from thread\n");
  return NULL;
}

int main() {
  pthread_t thread;
  pthread_create(&thread, NULL, thread_main, NULL);
  pthread_join(thread, NULL);
  printf("Hello from main\n");
  return 0;
}
