#include <assert.h>
#include <pthread.h>
#include <stdio.h>

void* thread_func(void* arg) {
  printf("Hello from thread\n");
  return NULL;
}

int main() {
  pthread_t t;
  pthread_create(&t, NULL, thread_func, NULL);
  assert(t);
  pthread_join(t, NULL);
  printf("done\n");
  return 0;
}
