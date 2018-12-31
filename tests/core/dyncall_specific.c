#include <emscripten.h>

void waka(int x, int y, int z) {
  EM_ASM({
    out('received ' + [$0, $1, $2] + '.');
  }, x, y, z);
}

int main() {
  EM_ASM({
#if DIRECT
    dynCall_viii($0, 1, 4, 9);
    return;
#endif
#if EXPORTED
    Module['dynCall_viii']($0, 1, 4, 9);
    return;
#endif
#if FROM_OUTSIDE
    eval("Module['dynCall_viii'](" + $0 + ", 1, 4, 9)");
    return;
#endif
    throw "no test mode";
  }, &waka);
}

