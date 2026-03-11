#include <emscripten.h>
#include <stdio.h>

int g_data;

EMSCRIPTEN_KEEPALIVE void* getDataPtr() {
  return &g_data;
}

int main() {
  EM_ASM({
    var ptr = ccall("getDataPtr", 'pointer');
    assert(ptr > 0);
  });
  printf("ok\n");
}
