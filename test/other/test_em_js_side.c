#include <emscripten/em_js.h>
#include <stdio.h>

EM_JS(void*, js_side_func_doubleasterisk, (int ptrsize, int num, char** ptr), {
  for (i=0;i<num;i++) {
    out(`argc=${i} == ${UTF8ToString(HEAP32[(ptr + (i * ptrsize)) >> 2])}`);
  }
  return 99;
});

EM_JS(void*, js_side_func, (int num, char* ptr), {
  out(`hello from side module ${num} + ${UTF8ToString(ptr)}`);
  return 99;
});

EM_JS(void, js_side_func2, (char *ptr), {
  out(UTF8ToString(ptr));
});

EM_JS(void, js_side_func_void, (), {
  out(`hello from void func`);
});

void test_side() {
  const int argc = 2;
  const char* argv[] = { "4", "2" }
  js_side_func_doubleasterisk(sizeof(char*), argc, (char**)argv);
  js_side_func(42, "hello");
  js_side_func2("hello again");
  js_side_func_void();
}
