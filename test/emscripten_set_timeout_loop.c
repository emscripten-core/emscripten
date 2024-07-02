#include <emscripten/emscripten.h>
#include <emscripten/html5.h>
#include <emscripten/em_asm.h>
#include <assert.h>
#include <stdlib.h>

double previousSetTimeouTime = 0;
int funcExecuted = 0;

void testDone(void *userData) {
  assert((long)userData == 2);
  assert(funcExecuted == 10);
  exit(0);
}

bool tick(double time, void *userData) {
  assert(time >= previousSetTimeouTime);
  previousSetTimeouTime = time;
  assert((long)userData == 1);
  ++funcExecuted;
  if (funcExecuted == 10)
  {
    emscripten_set_timeout(testDone, 300, (void*)2);
  }
  return funcExecuted < 10;
}

int main() {
  emscripten_set_timeout_loop(tick, 100, (void*)1);
  emscripten_exit_with_live_runtime();
  return 99;
}
