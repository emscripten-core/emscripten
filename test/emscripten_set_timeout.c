#include <emscripten/emscripten.h>
#include <emscripten/html5.h>
#include <assert.h>
#include <stdlib.h>
#include <stdio.h>

int func1Executed = 0;
int func2Executed = 0;

void func1(void *userData);

void func2(void *userData) {
  ++func2Executed;
  printf("func2: %d\n", func2Executed);
  assert((long)userData == 2);

  if (func2Executed == 1) {
    // Test canceling a setTimeout: register a callback but then cancel it immediately
    long id = emscripten_set_timeout(func1, 10, (void*)2);
    emscripten_clear_timeout(id);

    // Without this, the test will not exit correctly
    // https://github.com/emscripten-core/emscripten/issues/23763
    emscripten_runtime_keepalive_pop();

    emscripten_set_timeout(func2, 100, (void*)2);
  }

  if (func2Executed == 2) {
    assert(func1Executed == 1);
    printf("done\n");
    exit(0);
  }
}

void func1(void *userData) {
  printf("func1\n");
  assert((long)userData == 1);
  ++func1Executed;

  assert(func1Executed == 1);

  emscripten_set_timeout(func2, 100, (void*)2);
}

int main() {
  emscripten_set_timeout(func1, 100, (void*)1);
  return 99;
}
