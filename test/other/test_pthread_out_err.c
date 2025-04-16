#include <stdio.h>
#include <emscripten/console.h>

// Test that stdout/printf and emscripten JS logging functions (out()
// and err()) are interleaved as expected and all arrive at the console.
// See https://github.com/emscripten-core/emscripten/issues/14804
int main() {
  printf        ("printf 1\n");
  emscripten_out("out    1");
  emscripten_err("err    1");
  printf        ("printf 2\n");
  emscripten_out("out    2");
  emscripten_err("err    2");
  printf        ("printf 3\n");
  emscripten_out("out    3");
  emscripten_err("err    3");
  printf("done\n");
  return 0;
}
