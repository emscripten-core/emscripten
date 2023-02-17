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

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

_Atomic int thread_count = 0;
_Atomic int dso_count = 0;
void jslib_func();

void* thread_main(void* arg) {
  int num = (intptr_t)arg;
  printf("thread_main %d %p\n", num, pthread_self());
  thread_count++;

  void* func_addr = dlsym(RTLD_DEFAULT, "jslib_func");
  assert(func_addr);
  printf("jslib_func via dlsym: %p\n", func_addr);

  // busy wait until all threads are running
  while (thread_count != NUM_THREADS) {}

  char filename[255];
  sprintf(filename, "liblib%d.so", num);
  printf("loading %s\n", filename);
  void* handle = dlopen(filename, RTLD_NOW|RTLD_GLOBAL);
  printf("done loading %s (total=%d)\n", filename, ++dso_count);
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

  printf("done thread_main %d\n", num);
  return NULL;
}

int main() {
  printf("in main: %p\n", pthread_self());
  jslib_func();

  pthread_mutex_lock(&mutex);

  // start a bunch of threads while holding the lock
  pthread_t threads[NUM_THREADS];
  for (int i = 0; i < NUM_THREADS; i++) {
    pthread_create(&threads[i], NULL, thread_main, (void*)(intptr_t)i);
  }

  // busy wait until all threads are running
  while (thread_count != NUM_THREADS) {}

  for (int i = 0; i < NUM_THREADS; i++) {
    pthread_join(threads[i], NULL);
  }

  printf("main done\n");
  return 0;
}
