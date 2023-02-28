#include <assert.h>
#include <stdatomic.h>
#include <stdbool.h>
#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <emscripten/emscripten.h>

pthread_t t;

// This location should never get set to true.
// We verify that it false from JS after the program exits.
// If the main thread ever returns from `join` or the worker thread returns
// from `exit` this gets set to true, which would be bug.
atomic_bool fail = false;

EMSCRIPTEN_KEEPALIVE atomic_bool* fail_address() {
  return &fail;
}

void* thread_main_exit(void* arg) {
  // This test run with both _EXIT defined and without to test low level
  // and high level exit.
#ifdef _EXIT
  printf("calling _exit\n");
  _exit(43);
#else
  printf("calling exit\n");
  exit(42);
#endif
  fail = true;
  printf("after exit -- should never get here\n");
  __builtin_trap();
}

int main() {
  printf("main\n");
  int rc = pthread_create(&t, NULL, thread_main_exit, NULL);
  assert(rc == 0);
  void* thread_rtn = 0;
  rc = pthread_join(t, &thread_rtn);
  assert(rc == 0);
  // pthread_join should never return because the runtime should
  // exit first.
  fail = true;
  printf("done join %d -- should never get here\n", rc);
  __builtin_trap();
}
