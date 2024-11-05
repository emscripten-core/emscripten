#include <stdio.h>
#include <emscripten.h>
#include <emscripten/bind.h>

using namespace emscripten;

__attribute__((constructor))
void malloc_in_static_constructor(void) {
  // Malloc different sizes in AOT generation mode and normal mode to test
  // that method pointers still work correctly.
  EM_ASM(
    if (typeof InvokerFunctions == 'undefined') {
      _malloc(10);
    } else {
      _malloc(100);
    }
  );
}

class Foo {
public:
  void go() {}
};

int main() {
  printf("done\n");
}

EMSCRIPTEN_BINDINGS(xxx) {
    class_<Foo>("Foo")
      .function("go", &Foo::go);
}
