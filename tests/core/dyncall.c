#include <emscripten.h>

void waka(int x, int y, int z) {
  EM_ASM({
    Module['print']('received ' + [$0, $1, $2] + '.');
  }, x, y, z);
}

int main() {
  EM_ASM({
#if EXPORTED
    // test for additional things being exported
    assert(Module['addFunction']);
    // the main test here
    Module['dynCall']('viii', $0, [1, 4, 9]);
#else
    dynCall('viii', $0, [1, 4, 9]);
#endif
  }, &waka);
}

