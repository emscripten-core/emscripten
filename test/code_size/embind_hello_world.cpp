#include <string>
#include <emscripten/emscripten.h>
#include <emscripten/bind.h>

using namespace emscripten;

int main() {
  EM_ASM(out(Module.getString()));
}

std::string getString() {
  return "Hello, world";
}

EMSCRIPTEN_BINDINGS(typeOf) {
  function("getString", &getString);
}
