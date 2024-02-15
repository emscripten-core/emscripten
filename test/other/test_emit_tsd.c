#include <emscripten.h>

EMSCRIPTEN_KEEPALIVE void fooVoid() {}
EMSCRIPTEN_KEEPALIVE int fooInt(int a, int b) {
  return 42;
}

int main() {

}
