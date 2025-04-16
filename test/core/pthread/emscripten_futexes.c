#include <assert.h>
#include <emscripten/threading.h>
#include <errno.h>
#include <limits.h>
#include <stdio.h>

int main() {
  // This should return -EWOULDBLOCK.
  int futex_value = 42;
  int rc = emscripten_futex_wait(&futex_value, futex_value + 1, 0);
  assert(rc == -EWOULDBLOCK);

  // This should return -ETIMEDOUT.
  rc = emscripten_futex_wait(&futex_value, futex_value, 1);
  assert(rc == -ETIMEDOUT);

  // Check that this thread has removed itself from the wait queue.
  rc = emscripten_futex_wake(&futex_value, INT_MAX);
  assert(rc == 0);

  // Check that the wait address is checked for validity.
  void *bad_address = (void *) ~(uintptr_t) 0;
  rc = emscripten_futex_wake(bad_address, futex_value);
  // TODO: Should these return EFAULT instead?
  assert(rc == -EINVAL/*-EFAULT*/);
  rc = emscripten_futex_wake(NULL, 1);
  assert(rc == -EINVAL/*-EFAULT*/);

  printf("OK\n");
}
