#include <assert.h>
#include <pthread.h>
#include <stdlib.h>

static _Thread_local struct {
  pthread_mutex_t mutex;
  char padding[4096];
} tls;

void* leave_locked(void) {
  pthread_mutexattr_t attr;
  assert(pthread_mutexattr_init(&attr) == 0);
  assert(pthread_mutexattr_setrobust(&attr, PTHREAD_MUTEX_ROBUST) == 0);
  assert(pthread_mutex_init(&tls.mutex, &attr) == 0);
  assert(pthread_mutex_lock(&tls.mutex) == 0);

  // Keep the following block allocated so that freeing TLS cannot merge it
  // with the top of the heap. Leave the mutex locked for thread-exit cleanup.
  void* guard = malloc(32);
  assert(guard);
  return guard;
}
