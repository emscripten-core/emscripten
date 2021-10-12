#include <stdio.h>
#include <emscripten/em_asm.h>

void out(const char* msg) {
  EM_ASM({ out(UTF8ToString($0)); }, msg);
}

void err(const char* msg) {
  EM_ASM({ err(UTF8ToString($0)); }, msg);
}

// Test that stdout/printf and emscripten JS logging functions (out()
// and err()) are interleaved as expected and all arrive at the console.
// See https://github.com/emscripten-core/emscripten/issues/14804
int main() {
  printf("printf 1\n");
  out   ("out    1");
  err   ("err    1");
  printf("printf 2\n");
  out   ("out    2");
  err   ("err    2");
  printf("printf 3\n");
  out   ("out    3");
  err   ("err    3");
  printf("done\n");
  return 0;
}
