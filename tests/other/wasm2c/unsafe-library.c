#include <emscripten.h>
#include <stdio.h>

EMSCRIPTEN_KEEPALIVE
int twice(int x) {
  return x + x;
}

EMSCRIPTEN_KEEPALIVE
int do_bad_thing(int size) {
  return printf("I am in a sandbox and should not be able to print this!");
}
