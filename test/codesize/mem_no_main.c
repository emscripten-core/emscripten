#include <emscripten.h>
#include <stdlib.h>

int some_arg;

EMSCRIPTEN_KEEPALIVE
int foo() {
  return (int)(long)malloc(some_arg);
}
