#include <emscripten.h>

EM_JS(int, out_to_js, (int x), {})

class MyClass {
public:
  void foo();
  void bar();
};

void __attribute__((noinline)) MyClass::foo() {
  out_to_js(0); // line 12
  out_to_js(1);
  out_to_js(2);
}

void __attribute__((always_inline)) MyClass::bar() {
  out_to_js(3);
  __builtin_trap(); // line 19
}

int main() {
  MyClass mc;
  mc.foo();
  mc.bar();
}
