#include <assert.h>
#include <emscripten.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

void answer_callback(int ms, intptr_t value, void (*callback)(bool err, void* result, void* arg), void* arg);
void wait_ms_callback(int ms, void (*callback)(bool err, void* result, void* arg), void* arg);

int stage;
int returned;

void on_waited(bool err, void* result, void* arg) {
  assert(stage++ == 3);
  // A void function is called back with NULL.
  assert(!err);
  assert(result == NULL);
  assert(arg == &stage);
  printf("done\n");
  exit(0);
}

void on_rejected(bool err, void* result, void* arg) {
  assert(stage++ == 2);
  assert(err);
  assert(result == NULL);
  assert(arg == &stage);
  wait_ms_callback(5, on_waited, &stage);
}

void on_fulfilled(bool err, void* result, void* arg) {
  assert(stage++ == 1);
  assert(!err);
  assert((intptr_t)result == 42);
  assert(arg == &stage);
  // A rejected promise.
  answer_callback(5, 0, on_rejected, &stage);
}

void on_sync(bool err, void* result, void* arg) {
  assert(stage++ == 0);
  assert(returned);
  assert(!err);
  assert((intptr_t)result == 7);
  assert(arg == &stage);
  answer_callback(5, 42, on_fulfilled, &stage);
}

int main() {
  // A synchronous completion is still called back after the return.
  answer_callback(0, 7, on_sync, &stage);
  returned = 1;
  return 0;
}
