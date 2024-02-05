#include <assert.h>
#include <emscripten.h>
#include <stdio.h>

// Import the syscall under a separate name
__attribute__((import_module("env"), import_name("emscripten_get_now"))) double
__orig_emscripten_get_now(void);

double emscripten_get_now(void) {
  printf("emscripten_get_now intercepted\n");
  return __orig_emscripten_get_now();
}

int main() {
  double now = emscripten_get_now();
  assert(now > 0); // any non-negative time is valid
  return 0;
}
