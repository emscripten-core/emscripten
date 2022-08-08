#include <assert.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <emscripten/emscripten.h>

bool doneCallback = false;

void myatexit() {
  printf("myatexit\n");
  assert(doneCallback);
}

void callback(void *arg) {
  printf("callback\n");
  doneCallback = true;
  assert(arg == (void *)42);
  // Runtime should exit after this callback returns
}

int main() {
  atexit(myatexit);
  // The runtime should stay alive long enough for the callbackl
  // to run.
  emscripten_async_call(callback, (void*)42, 500);
  printf("returning from main\n");
  return 0;
}
