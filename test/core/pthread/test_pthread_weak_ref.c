#include <assert.h>
#include <pthread.h>
#include <stdbool.h>
#include <stdio.h>
#include <unistd.h>

pthread_barrier_t started;

void *worker_thread(void *arg) {
  printf("worker_thread\n");

  pthread_barrier_wait(&started);

  // Infinite loop
  while (1) {}

  return NULL;
}

int main() {
  pthread_t thread;

  printf("main\n");
  pthread_barrier_init(&started, NULL, 2);
  int rc = pthread_create(&thread, NULL, worker_thread, NULL);
  assert(rc == 0);

  // Wait until the thread executes its entry point
  pthread_barrier_wait(&started);

  printf("done\n");
  return 0;
}
