#include <assert.h>
#include <stdlib.h>
#include <dlfcn.h>
#include <stdio.h>

#include <emscripten/emscripten.h>
#include <emscripten/promise.h>

void load_side_module();

int load_count = 0;
bool in_callback = false;

em_promise_result_t on_fullfilled(void **result, void* data, void *handle) {
  printf("onsuccess: %d\n", load_count++);
  int* foo = (int*)dlsym(handle, "foo");
  assert(foo);
  printf("foo = %d\n", *foo);
  assert(*foo == 42);

  in_callback = true;
  if (load_count < 2) {
    // Load the same again, and make sure we don't re-enter ourselves.
    load_side_module();
  }
  in_callback = false;

  return EM_PROMISE_FULFILL;
}

em_promise_result_t on_rejected(void **result, void* data, void *value) {
  printf("onerror %s\n", dlerror());
  return EM_PROMISE_FULFILL;
}

void load_side_module() {
  em_promise_t inner = emscripten_dlopen_promise("libside.so", RTLD_NOW);
  em_promise_t outer = emscripten_promise_then(inner, on_fullfilled, on_rejected, NULL);
  emscripten_promise_destroy(outer);
  emscripten_promise_destroy(inner);
}

int main() {
  load_side_module();
  printf("returning from main\n");
  return 0;
}
