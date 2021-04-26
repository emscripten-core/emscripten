#include <stdio.h>
#include <emscripten.h>

int foo() {
  return 42;
}

EMSCRIPTEN_KEEPALIVE void say_hello() {
  printf("Hello! answer: %d\n", foo());
}
