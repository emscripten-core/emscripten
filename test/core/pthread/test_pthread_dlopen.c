#include <assert.h>
#include <dlfcn.h>
#include <pthread.h>
#include <stdio.h>
#include <unistd.h>

#define NUM_THREADS 8

typedef int* (*sidey_data_type)();
typedef int (*func_t)();
typedef func_t (*sidey_func_type)();

static sidey_data_type p_side_data_address;
static sidey_func_type p_side_func_address;
static int* expected_data_addr;
static func_t expected_func_addr;

pthread_barrier_t started;

static void* thread_main(void* arg) {
  int id = (int)(intptr_t)arg;

  printf("thread %d: in thread_main\n", id);

  // Wait until all threads + main have reached the barrier
  pthread_barrier_wait(&started);

  int* data_addr = p_side_data_address();
  assert(data_addr == expected_data_addr);

  func_t func_addr = p_side_func_address();
  assert(expected_func_addr == func_addr);
  assert(func_addr() == 43);

  printf("thread %d: thread_main done\n", id);
  return NULL;
}

int main() {
  printf("in main\n");

  pthread_barrier_init(&started, NULL, NUM_THREADS + 1);
  pthread_t threads[NUM_THREADS];

  // Start threads before loading the shared library
  for (int i = 0; i < NUM_THREADS; ++i) {
    int rc = pthread_create(&threads[i], NULL, thread_main, (void*)(intptr_t)i);
    assert(rc == 0);
  }

  printf("loading dylib\n");
  void* handle = dlopen("libside.so", RTLD_NOW|RTLD_GLOBAL);
  if (!handle) {
    printf("dlerror: %s\n", dlerror());
  }
  assert(handle);
  p_side_data_address = dlsym(handle, "side_data_address");
  printf("p_side_data_address=%p\n", p_side_data_address);
  p_side_func_address = dlsym(handle, "side_func_address");
  printf("p_side_func_address=%p\n", p_side_func_address);

  expected_data_addr = p_side_data_address();

  // side_func_address return the address of a function
  // internal to the side module (i.e. part of its static
  // table region).
  expected_func_addr = p_side_func_address();
  printf("p_side_func_address -> %p\n", expected_func_addr);
  assert(expected_func_addr() == 43);

  // Wait until all threads execute their entry points
  pthread_barrier_wait(&started);
  for (int i = 0; i < NUM_THREADS; ++i) {
    int rc = pthread_join(threads[i], NULL);
    assert(rc == 0);
  }

  dlclose(handle);

  printf("done\n");
  return 0;
}
