#include <assert.h>
#include <errno.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

pthread_mutex_t mtx;

void* original_owner_thread(void* ptr) {
  printf("[original owner] Setting lock...\n");
  pthread_mutex_lock(&mtx);
  printf("[original owner] Locked. Now exiting without unlocking.\n");
  pthread_exit(NULL);
}

int main(void) {
  pthread_mutexattr_t attr;
  pthread_mutexattr_init(&attr);
  pthread_mutexattr_setrobust(&attr, PTHREAD_MUTEX_ROBUST);

  pthread_mutex_init(&mtx, &attr);

  pthread_t thr;
  pthread_create(&thr, NULL, original_owner_thread, NULL);
  pthread_join(thr, NULL);

  printf("[main] Attempting to lock the robust mutex.\n");
  int rtn = pthread_mutex_lock(&mtx);
  printf("[main] pthread_mutex_lock -> %s.\n", strerror(rtn));
  assert(rtn == EOWNERDEAD && "expected EOWNERDEAD");
  printf("[main] Now make the mutex consistent\n");
  rtn = pthread_mutex_consistent(&mtx);
  assert(rtn == 0);
  printf("[main] Mutex is now consistent; unlocking\n");
  rtn = pthread_mutex_unlock(&mtx);
  assert(rtn == 0);

  return 0;
}
