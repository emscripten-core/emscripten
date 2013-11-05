#include <stdio.h>
#include <assert.h>
#include <emscripten.h>

int w1;

void c1(char *data, int size, void *arg) {
  assert((int)arg == 93);
  int *x = (int*)data;
  printf("c1: %d,%d\n", x[0], x[1]);

  int result = x[1] % x[0];
  REPORT_RESULT();
}

int main() {
  w1 = emscripten_create_worker("worker.js");

  int x[2] = { 100, 6002 };
  emscripten_call_worker(w1, "one", (char*)x, sizeof(x), c1, (void*)93);

  return 0;
}

