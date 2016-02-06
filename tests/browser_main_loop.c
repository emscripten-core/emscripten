#include <stdio.h>
#include <emscripten.h>

void main_loop(void) {
}

int main(void) {
  emscripten_set_main_loop(main_loop, 0, 0);
  return 0;
}

