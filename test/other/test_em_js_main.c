#include <stdio.h>
#include <emscripten.h>

int test_side();

int main() {
  printf("in main\n");
  return test_side();
}
