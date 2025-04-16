#include <assert.h>
#include <pthread.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond = PTHREAD_COND_INITIALIZER;
bool should_exit;

pthread_t thread;

void *workerThread(void* arg) {
  pthread_mutex_lock(&mutex);
  while (!should_exit)
    pthread_cond_wait(&cond, &mutex);
  pthread_mutex_unlock(&mutex);

  return NULL;
}

void terminateThread() {
  pthread_mutex_lock(&mutex);
  should_exit = true;
  pthread_cond_signal(&cond);
  pthread_mutex_unlock(&mutex);

  int res = 0;
  int rc = pthread_join(thread, (void**)&res);
  assert(rc == 0);
  assert(res == 0);

  printf("done waiting - thread successfully terminated\n");
}

int main(int argc, char* argv[]) {
  int rc = atexit(terminateThread);
  assert(rc == 0);

  rc = pthread_create(&thread, NULL, workerThread, NULL);
  assert(rc == 0);
  return 0;
}
