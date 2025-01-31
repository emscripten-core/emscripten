#include <emscripten/emscripten.h>
#include <stdio.h>

EM_JS_DEPS(deps, "$UTF8ToString");

int main() {
  char ch[256] = {};
  for (int i = 0; i < 255; ++i) {
    ch[i] = i+1;
  }
  int totalLen = 0;
  for (int i = 0; i < 256; ++i) {
    totalLen += EM_ASM_INT({return UTF8ToString($0).length}, ch);
  }
  printf("OK. Length: %d\n", totalLen);
}
