#include <emscripten.h>

namespace Namespace {

EM_JS(int, out_to_js, (int x), {})

class SomeClass{};

void __attribute__((noinline)) foo(SomeClass v) {
  out_to_js(0);
}

template <typename T>
void __attribute__((noinline)) bar(T t) {
  __builtin_trap();
}

}; // endof Namespace

int main() {
  Namespace::foo({});
  Namespace::bar(Namespace::SomeClass{});
  return 0;
}
