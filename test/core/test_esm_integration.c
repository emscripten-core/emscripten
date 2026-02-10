#include <stdio.h>
#include <emscripten.h>

EMSCRIPTEN_KEEPALIVE void foo() {
  printf("foo called\n");
}

int main(int argc, char* argv[]) {
  printf("hello, world! (%d)\n", argc);
  return 0;
}
