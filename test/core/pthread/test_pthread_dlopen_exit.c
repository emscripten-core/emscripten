#include <assert.h>
#include <dlfcn.h>
#include <pthread.h>
#include <stdio.h>

// NUM_SIDE_MODULES is passed on the command line and matches the number of
// libsideN.so copies created by the test runner. Each one is loaded while a
// worker thread is exiting, so it is also the number of worker threads.

pthread_barrier_t barrier;

void* exiting_worker(void* arg) {
  pthread_barrier_wait(&barrier);
  // Race thread shutdown against dynamic link synchronization.
  return NULL;
}

void* loader_thread(void* arg) {
  for (int i = 0; i < NUM_SIDE_MODULES; ++i) {
    char filename[32];
    snprintf(filename, sizeof(filename), "libside%d.so", i);
    // Pair each dlopen with the exit of one worker thread.
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

  // One worker per side module: the barrier pairs each worker with one
  // iteration of the loader thread, so the two loop counts must match.
  for (int i = 0; i < NUM_SIDE_MODULES; ++i) {
    pthread_t worker;
    assert(pthread_create(&worker, NULL, exiting_worker, NULL) == 0);
    assert(pthread_join(worker, NULL) == 0);
  }

  assert(pthread_join(loader, NULL) == 0);
  assert(pthread_barrier_destroy(&barrier) == 0);
  printf("done\n");
  return 0;
}
