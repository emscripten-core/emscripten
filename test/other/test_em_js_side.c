#include <emscripten/em_js.h>
#include <stdio.h>

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

EM_JS(void, js_side_func_indirect_ptr, (const char **pptr), {
  out(UTF8ToString(getValue(pptr, '*')));
});

int test_side() {
  const char *hello = "hello";
  js_side_func(42, "hello");
  js_side_func2("hello again");
  js_side_func_void();
  js_side_func_indirect_ptr(&hello);
  return 0;
}
