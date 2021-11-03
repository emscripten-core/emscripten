#include <emscripten/html5.h>
#include <stdlib.h>
#include <assert.h>
#include <stdio.h>

int func1Executed = 0;
int func2Executed = 0;

void func1(void *userData);

void func2(void *userData) {
  printf("func2: %p\n", userData);
  assert((long)userData == 2);
  ++func2Executed;

  if (func2Executed == 1) {
    // Test canceling a setImmediate: register a callback but then cancel it immediately
    long id = emscripten_set_immediate(func1, (void*)2);
    emscripten_clear_immediate(id);

    emscripten_set_timeout(func2, 100, (void*)2);
  }
  if (func2Executed == 2) {
    assert(func1Executed == 1);
    exit(0);
  }
}

void func1(void *userData) {
  printf("func1: %p\n", userData);
  assert((long)userData == 1);
  ++func1Executed;
  assert(func1Executed == 1);
  emscripten_set_immediate(func2, (void*)2);
}

int main() {
  emscripten_set_immediate(func1, (void*)1);
  return 99;
}
