#include <assert.h>
#include <dlfcn.h>
#include <pthread.h>
#include <stdio.h>

#ifndef NUM_THREADS
#define NUM_THREADS 2
#endif

typedef int* (*sidey_data_type)();
typedef int (*func_t)();
typedef func_t (*sidey_func_type)();

void jslib_func();

pthread_barrier_t started;

static void* thread_main(void* arg) {
  int id = (int)(intptr_t)arg;

  printf("thread %d: in thread_main\n", id);

  void* func_addr = dlsym(RTLD_DEFAULT, "jslib_func");
  assert(func_addr);
  printf("jslib_func via dlsym: %p\n", func_addr);

  // Wait until all threads + main have reached the barrier
  pthread_barrier_wait(&started);

  char filename[32];
  snprintf(filename, sizeof(filename), "libside%d.so", id);
  printf("loading %s\n", filename);
  void* handle = dlopen(filename, RTLD_NOW|RTLD_GLOBAL);
  printf("done loading %s (%d of %d)\n", filename, id + 1, NUM_THREADS);
  if (!handle) {
    printf("dlerror: %s\n", dlerror());
  }
  assert(handle);

  sidey_data_type p_side_data_address;
  sidey_func_type p_side_func_address;
  p_side_data_address = dlsym(handle, "side_data_address");
  printf("p_side_data_address=%p\n", p_side_data_address);
  p_side_func_address = dlsym(handle, "side_func_address");
  printf("p_side_func_address=%p\n", p_side_func_address);

  printf("thread %d: thread_main done\n", id);
  return NULL;
}

int main() {
  printf("in main\n");
  jslib_func();

  pthread_barrier_init(&started, NULL, NUM_THREADS + 1);
  pthread_t threads[NUM_THREADS];

  // Start a bunch of threads
  for (int i = 0; i < NUM_THREADS; ++i) {
    int rc = pthread_create(&threads[i], NULL, thread_main, (void*)(intptr_t)i);
    assert(rc == 0);
  }

  // Wait until all threads execute their entry points
  pthread_barrier_wait(&started);
  for (int i = 0; i < NUM_THREADS; ++i) {
    int rc = pthread_join(threads[i], NULL);
    assert(rc == 0);
  }

  printf("done\n");
  return 0;
}
