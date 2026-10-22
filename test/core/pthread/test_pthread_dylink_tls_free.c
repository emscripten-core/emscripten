#include <assert.h>
#include <dlfcn.h>
#include <malloc.h>
#include <pthread.h>
#include <stdio.h>

#define NUM_THREADS 100

typedef int (*touch_side_tls_type)(int);

static touch_side_tls_type touch_side_tls;

static void* thread_main(void* arg) {
  assert(touch_side_tls(1) == 1);
  return NULL;
}

static void run_threads(int count) {
  for (int i = 0; i < count; i++) {
    pthread_t thread;
    pthread_create(&thread, NULL, thread_main, NULL);
    pthread_join(thread, NULL);
  }
}

int main() {
  void* handle = dlopen("libside.so", RTLD_NOW);
  assert(handle);
  touch_side_tls = (touch_side_tls_type)dlsym(handle, "touch_side_tls");
  assert(touch_side_tls);

  run_threads(10);
  size_t before = mallinfo().uordblks;
  run_threads(NUM_THREADS);
  size_t after = mallinfo().uordblks;

  // Each thread allocates the side module's 4 KiB TLS block, which must be
  // freed when the thread exits.
  assert(after < before + NUM_THREADS * 4096 / 2);
  printf("done\n");
  return 0;
}
