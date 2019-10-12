#include <stdlib.h>
#include <emscripten.h>

EMSCRIPTEN_KEEPALIVE void* allocate_buffer(int x) {
  return malloc(x);
}
