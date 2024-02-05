#include <assert.h>
#include <stdatomic.h>
#include <stdbool.h>
#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <emscripten/emscripten.h>

pthread_t t;

void* thread_main_exit(void* arg) {
  printf("calling exit\n");
  exit(42);
}

// This location should never get set to true.
// We verify that it false from JS after the program exits.
atomic_bool join_returned = false;

EMSCRIPTEN_KEEPALIVE atomic_bool* join_returned_address() {
  return &join_returned;
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
  join_returned = true;
  printf("done join %d -- should never get here\n", rc);
  __builtin_trap();
}
