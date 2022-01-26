#include <emscripten.h>

EM_JS(int, out_to_js, (int x), {})

void foo() {
  out_to_js(0); // line 5
  out_to_js(1); // line 6
  out_to_js(2); // line 7
  // A silly possible recursion to avoid binaryen doing any inlining.
  if (out_to_js(3)) foo();
}

int main() {
  foo();
}
