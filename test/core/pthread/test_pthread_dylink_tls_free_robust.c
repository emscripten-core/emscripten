#include <assert.h>
#include <dlfcn.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

static void* (*leave_locked)(void);

static void* thread_main(void* arg) { return leave_locked(); }

int main() {
  void* handle = dlopen("libside.so", RTLD_NOW);
  assert(handle);
  leave_locked = dlsym(handle, "leave_locked");
  assert(leave_locked);

  for (int i = 0; i < 10; ++i) {
    pthread_t thread;
    assert(pthread_create(&thread, NULL, thread_main, NULL) == 0);
    void* guard;
    assert(pthread_join(thread, &guard) == 0);

    // Robust mutex cleanup must finish before freeing the TLS containing the
    // mutex. Otherwise it writes into the freed block and corrupts the heap.
    volatile char* block = malloc(4096);
    assert(block);
    block[0] = 0x55;
    free((void*)block);
    free(guard);
  }

  puts("done");
  return 0;
}
