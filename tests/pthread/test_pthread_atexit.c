#include <assert.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

pthread_mutex_t count_lock = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t count_nonzero = PTHREAD_COND_INITIALIZER;
unsigned count;

pthread_t thread;

void *workerThread(void* arg) {
  pthread_mutex_lock(&count_lock);
  while (count == 0)
    pthread_cond_wait(&count_nonzero, &count_lock);
  count--;
  pthread_mutex_unlock(&count_lock);

  return NULL;
}

void terminateThread() {
  pthread_mutex_lock(&count_lock);
  if (count == 0)
    pthread_cond_signal(&count_nonzero);
  count++;
  pthread_mutex_unlock(&count_lock);

  int res = 0;
  int rc = pthread_join(thread, (void**)&res);
  assert(rc == 0);
  assert(res == 0);

  printf("done waiting - counter is: %d\n", count);
}

int main(int argc, char* argv[]) {
  int rc = atexit(terminateThread);
  assert(rc == 0);

  rc = pthread_create(&thread, NULL, workerThread, NULL);
  assert(rc == 0);
  return 0;
}
