#include <emscripten/atomic.h>
#include <pthread.h>
#include <assert.h>
#include <stdio.h>

// Test emscripten_atomics_is_lock_free() functions

void test() {
  assert(emscripten_atomics_is_lock_free(1));
  assert(emscripten_atomics_is_lock_free(2));
  assert(emscripten_atomics_is_lock_free(4));
  // Chrome is buggy, see
  // https://bugs.chromium.org/p/chromium/issues/detail?id=1167449
  //assert(emscripten_atomics_is_lock_free(8));
  assert(!emscripten_atomics_is_lock_free(31));
}

void* thread_main(void* arg) {
  test();
  return NULL;
}

int main() {
  test();
  pthread_t t;
  pthread_create(&t, NULL, thread_main, NULL);
  pthread_join(t, NULL);
  printf("done\n");
  return 0;
}
