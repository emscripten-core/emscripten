#include <emscripten/html5.h>
#include <assert.h>
#include <stdlib.h>

int funcExecuted = 0;

void testDone(void *userData) {
  assert((long)userData == 2);
  assert(funcExecuted == 10);
  exit(0);
}

bool tick(void *userData) {
  assert((long)userData == 1);
  ++funcExecuted;
  if (funcExecuted == 10) {
    emscripten_set_timeout(testDone, 300, (void*)2);
  }
  return funcExecuted < 10;
}

int main() {
  emscripten_set_immediate_loop(tick, (void*)1);
  return 99;
}
