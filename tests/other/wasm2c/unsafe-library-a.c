#include <emscripten.h>

EMSCRIPTEN_KEEPALIVE
int twice(int x) {
  return x + x;
}
