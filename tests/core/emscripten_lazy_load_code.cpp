#include <emscripten.h>
#include <stdio.h>
#include <stdlib.h>

extern "C" {

void foo_start(int n) {
  puts("foo_start");
  // prevent inlining
  if (n > 10000) foo_start(n - 1);
  if (n > 15000) foo_start(n - 3);
}

void foo_end(int n) {
  puts("foo_end");
  // prevent inlining
  if (n > 20000) foo_end(n - 2);
  if (n > 30000) foo_end(n - 5);
}

} // extern "C"

int main(int argc, char** argv) {
  int x = atoi(argv[1]);
  foo_start(x);
  if (x != 42) {
    emscripten_lazy_load_code();
  }
  foo_end(x); // this can be elided in the first download, only needed after we lazily load the second
}
