#include <stdio.h>
#include <emscripten/emscripten.h>
#include <emscripten/bind.h>

using namespace emscripten;

class A {
public:
  int i = 2;
};

class B {
public:
  A a;
};

EMSCRIPTEN_BINDINGS(property) {
  class_<A>("A").constructor().property("i", &A::i);
  class_<B>("B").constructor().property("a", &B::a);
}

int main() {
  EM_ASM({
    // Test persistence assigning fields of the field.
    var b = new Module.B();
    b.a.i = 99;
    console.log("b.a.i = %d", b.a.i);

    // Test assignment into the field.
    var a = new Module.A();
    a.i = 101;
    b.a = a;
    console.log("b.a.i = %d", b.a.i);

    // Change the sub field and ensure `b.a` and `a` are separate.
    a.i = 103;
    console.log("b.a.i = %d", b.a.i);
    console.log("a.i = %d", a.i);

    a.delete();
    b.delete();
  });
  return 0;
}
