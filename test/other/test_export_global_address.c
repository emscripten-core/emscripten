#include <emscripten.h>
#include <assert.h>
#include <stdio.h>

#ifdef USE_KEEPALIVE
EMSCRIPTEN_KEEPALIVE
#endif
int g_foo = 4;

EM_JS(int*, get_foo_from_js, (void), {
  assert(_g_foo !== undefined, "g_foo not exported to JS");
  return _g_foo;
});

int main() {
  printf("get_foo_from_js: %d\n", *get_foo_from_js());
  printf("g_foo: %d\n", g_foo);
  if (get_foo_from_js() != &g_foo) {
    printf("addresses failed to match\n");
    printf("js: %p\n", get_foo_from_js());
    printf("native: %p\n", &g_foo);
    return 1;
  }
  return 0;
}
