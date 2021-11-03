#include <emscripten.h>
#include <emscripten/html5.h>
#include <assert.h>
#include <stdlib.h>
#include <stdio.h>

int funcExecuted = 0;

void testDone(void *userData) {
  assert((long)userData == 2);
  assert(funcExecuted == 10);
  exit(0);
}

long intervalId = 0;

void tick(void *userData) {
  assert((long)userData == 1);
  ++funcExecuted;
  if (funcExecuted == 10) {
    emscripten_set_timeout(testDone, 300, (void*)2);
  }
  if (funcExecuted >= 10) {
    printf("clearing interval..\n");
    emscripten_clear_interval(intervalId);
  }
}

int main() {
  intervalId = emscripten_set_interval(tick, 100, (void*)1);
  emscripten_exit_with_live_runtime();
  return 99;
}
