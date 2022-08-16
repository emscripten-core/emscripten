#include <stdio.h>
#include <dlfcn.h>
#include <emscripten.h>

EMSCRIPTEN_KEEPALIVE void say_hello() {
  printf("Hello from main!\n");
  void *lib_handle = dlopen("libhello.wasm", RTLD_NOW);
  if (!lib_handle) {
    printf("Error opening libhello.wasm\n");
    return;
  }
  void (*lib_say_hello)() = (void(*)())dlsym(lib_handle, "lib_say_hello");
  if (!lib_say_hello) {
    printf("Error finding lib_say_hello function\n");
    return;
  }
  lib_say_hello();
}
