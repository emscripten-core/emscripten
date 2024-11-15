#include <cstdint>
#include <emscripten.h>
#include <emscripten/bind.h>

int64_t getBigInt() {
  return 1000000000000;
}

uint64_t getBigUInt() {
  return -1000000000000;
}

int main() {
  EM_ASM(
    console.log(Module.getBigInt());
    console.log(Module.getBigUInt());
  );
}

EMSCRIPTEN_BINDINGS(my_module) {
  emscripten::function("getBigInt", &getBigInt);
  emscripten::function("getBigUInt", &getBigUInt);
}
