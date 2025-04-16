#include <assert.h>
#include <stdlib.h>
#include <dlfcn.h>
#include <stdio.h>

#include <emscripten/emscripten.h>
#include <emscripten/promise.h>

em_promise_result_t on_fullfilled(void **result, void* data, void *handle) {
  printf("onsuccess\n");
  int* foo = (int*)dlsym(handle, "foo");
  assert(foo);
  printf("foo = %d\n", *foo);
  assert(*foo == 42);
  return EM_PROMISE_FULFILL;
}

em_promise_result_t on_rejected(void **result, void* data, void *value) {
  printf("onerror %s\n", dlerror());
  return EM_PROMISE_FULFILL;
}

int main() {
  em_promise_t inner = emscripten_dlopen_promise("libside.so", RTLD_NOW);
  em_promise_t outer = emscripten_promise_then(inner, on_fullfilled, on_rejected, NULL);
  emscripten_promise_destroy(outer);
  emscripten_promise_destroy(inner);
  printf("returning from main\n");
  return 0;
}
