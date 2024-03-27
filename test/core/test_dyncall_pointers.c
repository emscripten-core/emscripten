#include <stdio.h>
#include <emscripten/em_js.h>

EM_JS_DEPS(deps, "$dynCall");

EM_JS(void, testDynCall, (void* ptr, const char* str), {
  let rtn = dynCall("pipp", Number(ptr), [42, 43, Number(str)]);
  // Check that the returned pointer is unsigned.
  assert(rtn > 0, rtn);
  out("dynCall => " + UTF8ToString(rtn));
});

const char* foo(int a1, size_t a2, const char* a3) {
  printf("foo called: %d %zu '%s'\n", a1, a2, a3);
  return a3;
}

int main() {
  printf("in main\n");
  testDynCall(foo, "a string");
}
