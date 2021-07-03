#include <emscripten/html5.h>
#include <stdio.h>
#include <stdlib.h>

void timeout(void *userData) {
  printf("Got timeout handler\n");
  // Test passed
  exit(0);
}

int main() {
  emscripten_set_timeout(timeout, 2000, 0);
  emscripten_unwind_to_js_event_loop();
  // emscripten_unwind_to_js_event_loop should never return
  __builtin_unreachable();
}
