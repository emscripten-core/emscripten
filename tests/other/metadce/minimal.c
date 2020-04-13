#include <emscripten.h>

EMSCRIPTEN_KEEPALIVE int add(int x, int y) {
  return x + y;
}
