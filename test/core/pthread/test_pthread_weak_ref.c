#include <assert.h>
#include <pthread.h>
#include <stdbool.h>
#include <stdio.h>
#include <unistd.h>

pthread_cond_t cond = PTHREAD_COND_INITIALIZER;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

bool running = false;

void *worker_thread(void *arg) {
  printf("worker_thread\n");

  pthread_mutex_lock(&mutex);
  running = true;
  pthread_cond_signal(&cond);
  pthread_mutex_unlock(&mutex);

  // Infinite loop
  while (1) {}

  return NULL;
}

int main() {
  pthread_t thread;

  printf("main\n");
  int rc = pthread_create(&thread, NULL, worker_thread, NULL);
  assert(rc == 0);

  pthread_mutex_lock(&mutex);

  // Wait until the thread executes its entry point
  while (!running) {
    pthread_cond_wait(&cond, &mutex);
  }

  pthread_mutex_unlock(&mutex);

  printf("done\n");
  return 0;
}
