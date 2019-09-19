#include <emscripten.h>
#include <stdio.h>

extern "C" void foo_end(int n);

extern "C" EMSCRIPTEN_KEEPALIVE void foo_start(int n) {
  puts("foo_start");
  // prevent inlining
  if (n > 100) foo_end(n - 1);
  if (n > 150) foo_end(n - 3);
}

extern "C" EMSCRIPTEN_KEEPALIVE void foo_end(int n) {
  puts("foo_end");
  // prevent inlining
  if (n > 200) foo_start(n - 2);
  if (n > 300) foo_start(n - 5);
}

int main(int argc, char** argv) {
  foo_start(argc);
  emscripten_lazy_load_code();
  foo_end(argc); // this can be elided in the first download, only needed after we lazily load the second
}
