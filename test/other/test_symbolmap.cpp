#include <emscripten.h>

namespace Namespace {

EM_JS(int, out_to_js, (int x), {})

class ClassA{};
class ClassB{};

void __attribute__((noinline)) foo(ClassA v) {
  out_to_js(0);
}

template <typename T>
void __attribute__((noinline)) bar(ClassB t) {
  __builtin_trap();
}

}; // endof Namespace

int main() {
  Namespace::foo({});
  Namespace::bar<Namespace::ClassA>(Namespace::ClassB{});
  return 0;
}
