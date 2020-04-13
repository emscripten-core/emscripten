#include <stdio.h>
#include <emscripten.h>

EMSCRIPTEN_KEEPALIVE void c_func(char *str) {
  printf("%s\n", str);
}

EM_JS(void, js_func, (void), {
  _c_func(allocateUTF8OnStack('Hello, World!'));
});

int main(void) {
  js_func();
}
