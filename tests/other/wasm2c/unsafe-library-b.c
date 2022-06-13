#include <emscripten.h>

EMSCRIPTEN_KEEPALIVE
int thrice(int x) {
  return x + x + x;
}
