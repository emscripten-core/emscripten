#include <assert.h>
#include <pthread.h>
#include <stdatomic.h>
#include <stdio.h>

#include <emscripten/atomic.h>

// Test the various `is_lock_free` functions:
//
// - emscripten_atomics_is_lock_free
// - __atomic_always_lock_free
// - atomic_is_lock_free

void test() {
  assert(emscripten_atomics_is_lock_free(1));
  assert(emscripten_atomics_is_lock_free(2));
  assert(emscripten_atomics_is_lock_free(4));
  assert(emscripten_atomics_is_lock_free(8));
  assert(!emscripten_atomics_is_lock_free(16));
  assert(!emscripten_atomics_is_lock_free(31));

  // Test compiler buildin __atomic_always_lock_free version
  assert(__atomic_always_lock_free(1, 0));
  assert(__atomic_always_lock_free(2, 0));
  assert(__atomic_always_lock_free(4, 0));
  assert(__atomic_always_lock_free(8, 0));
  assert(!__atomic_always_lock_free(16, 0));
  assert(!__atomic_always_lock_free(31, 0));

  // Test C11 atomic_is_lock_free
  struct { char a[1]; } one;
  struct { char a[2]; } two;
  struct { char a[4]; } four;
  struct { char a[8]; } eight;
  struct { char a[16]; } sixteen;
  struct { char a[31]; } thirty_one;
  assert(atomic_is_lock_free(&one));
  assert(atomic_is_lock_free(&two));
  assert(atomic_is_lock_free(&four));
  assert(atomic_is_lock_free(&eight));
  assert(!atomic_is_lock_free(&sixteen));
  assert(!atomic_is_lock_free(&thirty_one));
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
