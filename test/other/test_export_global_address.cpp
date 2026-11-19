#include <emscripten.h>
#include <assert.h>
#include <stdio.h>

#ifdef USE_KEEPALIVE
#define KEEPALIVE EMSCRIPTEN_KEEPALIVE
#else
#define KEEPALIVE
#endif

extern "C" {
KEEPALIVE int g_var = 4;

KEEPALIVE int g_func(int x) {
  return x + g_var;
}
}

namespace ns {
KEEPALIVE int ns_var = 42;
}

KEEPALIVE int cpp_func(int x) {
  return x * 2;
}

EM_JS(int*, get_var_from_js, (void), {
  assert(_g_var !== undefined, "g_var not exported to JS");
#if __wasm64__
  return BigInt(_g_var);
#else
  return _g_var;
#endif
});

EM_JS(int, call_func_from_js, (int arg), {
  assert(_g_func !== undefined, "g_func not exported to JS");
  return _g_func(arg);
});

EM_JS(int*, get_ns_var_from_js, (void), {
  assert(__ZN2ns6ns_varE !== undefined, "ns::ns_var not exported to JS");
#if __wasm64__
  return BigInt(__ZN2ns6ns_varE);
#else
  return __ZN2ns6ns_varE;
#endif
});

EM_JS(int, call_cpp_func_from_js, (int arg), {
  assert(__Z8cpp_funci !== undefined, "cpp_func not exported to JS");
  return __Z8cpp_funci(arg);
});

int main() {
  printf("get_var_from_js: %d\n", *get_var_from_js());
  printf("g_var: %d\n", g_var);
  if (get_var_from_js() != &g_var) {
    printf("addresses failed to match\n");
    printf("js: %p\n", get_var_from_js());
    printf("native: %p\n", &g_var);
    return 1;
  }
  printf("call_func_from_js: %d\n", call_func_from_js(10));

  printf("get_ns_var_from_js: %d\n", *get_ns_var_from_js());
  printf("ns_var: %d\n", ns::ns_var);
  if (get_ns_var_from_js() != &ns::ns_var) {
    printf("ns_var addresses failed to match\n");
    printf("js: %p\n", get_ns_var_from_js());
    printf("native: %p\n", &ns::ns_var);
    return 1;
  }
  printf("call_cpp_func_from_js: %d\n", call_cpp_func_from_js(5));

  return 0;
}
