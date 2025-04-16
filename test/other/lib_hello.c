#include <stdio.h>
#include <emscripten.h>

EMSCRIPTEN_KEEPALIVE void lib_say_hello() {
  printf("Hello from lib!\n");
}
