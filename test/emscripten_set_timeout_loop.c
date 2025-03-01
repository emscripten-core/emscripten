#include <emscripten/emscripten.h>
#include <emscripten/html5.h>
#include <assert.h>
#include <stdlib.h>
#include <stdio.h>

double previousSetTimeouTime = 0;
int funcExecuted = 0;

void testDone(void *userData) {
  printf("testDone\n");
  assert((long)userData == 2);
  assert(funcExecuted == 10);
  emscripten_runtime_keepalive_pop();
  exit(0);
}

bool tick(double time, void *userData) {
  printf("tick: %d\n", funcExecuted);
  assert(time >= previousSetTimeouTime);
  previousSetTimeouTime = time;
  assert((long)userData == 1);
  ++funcExecuted;
  if (funcExecuted == 10) {
    emscripten_set_timeout(testDone, 300, (void*)2);
  }
  return funcExecuted < 10;
}

int main() {
  emscripten_set_timeout_loop(tick, 100, (void*)1);
  emscripten_runtime_keepalive_push();
  return 99;
}
