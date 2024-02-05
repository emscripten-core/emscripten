#include <emscripten.h>

EM_JS(int, out_to_js, (int x), {})

void __attribute__((noinline)) foo() {
  out_to_js(0); // line 6
  out_to_js(1);
  out_to_js(2);
}

void __attribute__((always_inline)) bar() {
  out_to_js(3);
  __builtin_trap(); // line 13
}

int main() {
  foo();
  bar();
}
