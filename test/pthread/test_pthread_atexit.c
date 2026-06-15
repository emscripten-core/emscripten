#include <assert.h>
#include <pthread.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

pthread_barrier_t exit_barrier;

pthread_t thread;

void *workerThread(void* arg) {
  pthread_barrier_wait(&exit_barrier);
  printf("done workerThread\n");
  return NULL;
}

void terminateThread() {
  printf("terminateThread\n");
  pthread_barrier_wait(&exit_barrier);

  int res = 0;
  int rc = pthread_join(thread, (void**)&res);
  assert(rc == 0);
  assert(res == 0);

  printf("done waiting - thread successfully terminated\n");
}

int main(int argc, char* argv[]) {
  pthread_barrier_init(&exit_barrier, NULL, 2);
  int rc = atexit(terminateThread);
  assert(rc == 0);

  rc = pthread_create(&thread, NULL, workerThread, NULL);
  assert(rc == 0);
  printf("done main\n");
  return 0;
}
