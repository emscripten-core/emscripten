#include <assert.h>
#include <errno.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <emscripten/heap.h>

// The thread block comes from emscripten_builtin_malloc, which this does not
// override, so setting fail_malloc fails only the allocations after it.
static _Atomic int fail_malloc;

void* malloc(size_t size) {
  if (fail_malloc) {
    return NULL;
  }
  return emscripten_builtin_malloc(size);
}

void free(void* ptr) {
  emscripten_builtin_free(ptr);
}

void* thread_main(void* arg) {
  return arg;
}

int main() {
  pthread_t t = 0;
  pthread_attr_t attr;
  void* result;

  // A stack larger than the heap makes the thread block allocation fail.
  pthread_attr_init(&attr);
  pthread_attr_setstacksize(&attr, 64 * 1024 * 1024);
  int rtn = pthread_create(&t, &attr, thread_main, NULL);
  pthread_attr_destroy(&attr);
  printf("block: %d\n", rtn == EAGAIN);
  assert(t == 0);

  // The thread block fits, but the mailbox allocation fails.
  fail_malloc = 1;
  rtn = pthread_create(&t, NULL, thread_main, NULL);
  fail_malloc = 0;
  printf("mailbox: %d\n", rtn == EAGAIN);
  assert(t == 0);

  // The heap and the main thread are intact.
  free(malloc(1024));
  rtn = pthread_create(&t, NULL, thread_main, (void*)42);
  assert(rtn == 0);
  rtn = pthread_join(t, &result);
  assert(rtn == 0);
  assert(result == (void*)42);
  printf("done\n");
  return 0;
}
