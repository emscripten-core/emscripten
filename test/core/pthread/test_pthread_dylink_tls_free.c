#include <assert.h>
#include <dlfcn.h>
#include <malloc.h>
#include <pthread.h>
#include <stdio.h>

#define NUM_THREADS 100

typedef int (*touch_side_tls_type)(int);

// Kept static so it does not clash with the side module's exported
// touch_side_tls symbol under MAIN_MODULE.
static touch_side_tls_type touch_side_tls;

void* thread_main(void* arg) {
  assert(touch_side_tls(1) == 1);
  return NULL;
}

void run_threads(int count) {
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

  // Each thread allocates the side module's 4 KiB TLS block on first access and
  // must free it when it exits.  Without the fix the heap grows by one block
  // per thread; with it, growth is bounded by the last thread's not-yet-
  // reclaimed stack, far below NUM_THREADS blocks.  NUM_THREADS is large so a
  // real leak clearly exceeds that fixed slack.
  run_threads(10);
  size_t before = mallinfo().uordblks;
  run_threads(NUM_THREADS);
  size_t after = mallinfo().uordblks;
  assert(after < before + NUM_THREADS * 4096 / 2);

  printf("done\n");
  return 0;
}
