#include <stdio.h>
#include <stdlib.h>

#include <emscripten/emscripten.h>
#include <emscripten/console.h>

void during_exit() {
  // An attempt to sleep during exit, which is not allowed (the exit process is
  // done in synchronous JS).
  emscripten_out("during_exit 1");
#ifndef NO_ASYNC
  emscripten_sleep(100);
#endif
  emscripten_out("during_exit 2");
}

int main() {
  atexit(&during_exit);
  return 0;
}
