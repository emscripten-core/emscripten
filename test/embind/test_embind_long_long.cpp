#include <cstdint>
#include <emscripten.h>
#include <emscripten/bind.h>

int64_t getInt64() {
  return 1000000000000;
}

uint64_t getUint64() {
  return -1000000000000;
}

int main() {
  EM_ASM(
    console.log(Module.getInt64());
    console.log(Module.getUint64());
  );
}

EMSCRIPTEN_BINDINGS(my_module) {
  emscripten::function("getInt64", &getInt64);
  emscripten::function("getUint64", &getUint64);
}
