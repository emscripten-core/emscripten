#include <stdio.h>
#include <emscripten/val.h>

using namespace emscripten;

int main() {
  val Math = val::global("Math");

  // two ways to call Math.abs
  printf("abs(-10): %d\n", Math.call<int>("abs", -10));
  printf("abs(-11): %d\n", Math["abs"](-11).as<int>());

  // Const-qualification and references should not matter.
  int x = -12;
  const int y = -13;
  printf("abs(%d): %d\n", x, Math.call<int>("abs", x)); // x is deduced to int&
  printf("abs(%d): %d\n", y, Math.call<int>("abs", y)); // y is deduced to const int&
  printf("abs(-14): %d\n", Math.call<const int>("abs", -14));

  return 0;
}
