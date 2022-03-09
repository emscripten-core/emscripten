#include <emscripten.h>

EM_JS(int, out_to_js, (int x), {})

void __attribute__((noinline)) foo() {
  out_to_js(0); // line 6
  out_to_js(1); // line 7
  out_to_js(2); // line 8
  // A silly possible recursion to avoid binaryen doing any inlining.
  if (out_to_js(3)) foo();
}

void __attribute__((always_inline)) bar() {
  out_to_js(3);
  __builtin_trap();
}

int main() {
  foo();
  bar();
}
