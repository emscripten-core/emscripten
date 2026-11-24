#define _GNU_SOURCE
#include <assert.h>
#include <dlfcn.h>
#include <stdio.h>

#include <emscripten/emscripten.h>

EMSCRIPTEN_KEEPALIVE int foo() { return 42; }

int main() {
  // RTLD_NEXT is not supported, but looking it up should fail cleanly rather
  // than abort.
  void* f = dlsym(RTLD_NEXT, "foo");
  assert(f == NULL);
  const char* err = dlerror();
  assert(err);
  printf("dlsym(RTLD_NEXT): %s\n", err);

  int (*g)() = dlsym(RTLD_DEFAULT, "foo");
  assert(g);
  assert(g() == 42);
  printf("done\n");
  return 0;
}
