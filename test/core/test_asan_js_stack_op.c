#include <stdio.h>
#include <emscripten.h>

EMSCRIPTEN_KEEPALIVE void c_func(char *str) {
  printf("%s\n", str);
}

EM_JS_DEPS(js_func, "$stringToUTF8OnStack");

EM_JS(void, js_func, (void), {
  var addr = stringToUTF8OnStack('Hello, World!');
#ifdef __wasm64__
  addr = BigInt(addr);
#endif
  _c_func(addr);
});

int main(void) {
  js_func();
}
