#include <assert.h>
#include <dlfcn.h>
#include <pthread.h>
#include <stdio.h>

static pthread_barrier_t barrier;

static void* exiting_worker(void* arg) {
  pthread_barrier_wait(&barrier);
  // Race thread shutdown against dynamic link synchronization.
  return NULL;
}

static void* loader_thread(void* arg) {
  for (int i = 0; i < NUM_ITERATIONS; ++i) {
    char filename[32];
    snprintf(filename, sizeof(filename), "libside%d.so", i);
    // Pair with every exiting worker, not just the first one.
    pthread_barrier_wait(&barrier);
    void* handle = dlopen(filename, RTLD_NOW | RTLD_GLOBAL);
    assert(handle && "dlopen failed");
    int (*side_func)(void) = dlsym(handle, "side_func");
    assert(side_func);
    assert(side_func() == 42);
    assert(dlclose(handle) == 0);
  }
  return NULL;
}

int main() {
  assert(pthread_barrier_init(&barrier, NULL, 2) == 0);

  pthread_t loader;
  assert(pthread_create(&loader, NULL, loader_thread, NULL) == 0);

  for (int i = 0; i < NUM_ITERATIONS; ++i) {
    pthread_t worker;
    assert(pthread_create(&worker, NULL, exiting_worker, NULL) == 0);
    assert(pthread_join(worker, NULL) == 0);
  }

  assert(pthread_join(loader, NULL) == 0);
  assert(pthread_barrier_destroy(&barrier) == 0);
  puts("PASSED");
  return 0;
}
