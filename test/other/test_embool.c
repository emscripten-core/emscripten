#include <stdio.h>
#include <emscripten/emscripten.h>

int main() {
  EM_BOOL b = EM_TRUE;
  printf("EM_TRUE:%d,EM_FALSE:%d\n", b, EM_FALSE);
  return 0;
}
