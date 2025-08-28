#include <emscripten.h>

EM_JS(int, out_to_js, (), {
  out(new Error().stack);
  return 0;
});

namespace Namespace {

class ClassA{};
class ClassB{};

void __attribute__((noinline)) foo(ClassA v) {
  out_to_js();
}

template <typename T>
void __attribute__((noinline)) bar(ClassB t) {
  __builtin_trap();
}

}; // endof Namespace

EMSCRIPTEN_KEEPALIVE
extern "C" void middle() {
  if (out_to_js()) {
    // fake recursion that is never reached, to avoid inlining in binaryen and LLVM
    middle();

    Namespace::foo({});
    Namespace::bar<Namespace::ClassA>(Namespace::ClassB{});
  }
}

int main() {
  EM_ASM({ _middle() });
  return 0;
}
