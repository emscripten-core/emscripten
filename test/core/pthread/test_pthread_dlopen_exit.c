#include <assert.h>
#include <dlfcn.h>
#include <pthread.h>
#include <stdbool.h>
#include <stdio.h>

// Each dlopen makes the main runtime thread queue a dlsync task for every live
// thread. Keep threads exiting while libraries are loaded, so that threads
// regularly exit with dlsync tasks still queued and have to cancel them.

#define NUM_CHURN_THREADS 6

_Atomic bool loading = true;

void* exiting_thread(void* arg) {
  return NULL;
}

void* churn_thread(void* arg) {
  while (loading) {
    pthread_t t;
    assert(pthread_create(&t, NULL, exiting_thread, NULL) == 0);
    assert(pthread_join(t, NULL) == 0);
  }
  return NULL;
}

int main() {
  pthread_t churners[NUM_CHURN_THREADS];
  for (int i = 0; i < NUM_CHURN_THREADS; ++i) {
    assert(pthread_create(&churners[i], NULL, churn_thread, NULL) == 0);
  }

  // Each libsideN.so is a separate copy, so every dlopen does real work.
  for (int i = 0; i < NUM_SIDE_MODULES; ++i) {
    char filename[32];
    snprintf(filename, sizeof(filename), "libside%d.so", i);
    void* handle = dlopen(filename, RTLD_NOW | RTLD_GLOBAL);
    assert(handle);
    int (*side_func)(void) = dlsym(handle, "side_func");
    assert(side_func);
    assert(side_func() == 42);
  }

  loading = false;
  for (int i = 0; i < NUM_CHURN_THREADS; ++i) {
    assert(pthread_join(churners[i], NULL) == 0);
  }
  printf("done\n");
  return 0;
}
