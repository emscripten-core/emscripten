// Copyright 2025 The Emscripten Authors.  All rights reserved.
// Emscripten is available under two separate licenses, the MIT license and the
// University of Illinois/NCSA Open Source License.  Both these licenses can be
// found in the LICENSE file.

#include <assert.h>
#include <emscripten.h>
#include <stdio.h>
#include <dlfcn.h>

EM_ASYNC_JS(int, test_suspending, (), {
  console.log("sleeping");
  await new Promise(res => setTimeout(res, 0));
  console.log("slept");
  return 77;
});

int test_suspending_wrapper() {
  return test_suspending();
}

EM_JS(int, test_sync, (), {
  console.log("sync");
  return 77;
})

int test_sync_wrapper() {
  return test_sync();
}


typedef int (*F)();
typedef int (*G)(F f);

void helper(F f) {
  void* handle = dlopen("side_a.so", RTLD_NOW|RTLD_GLOBAL);
  assert(handle != NULL);
  G side_module_trampolinea = dlsym(handle, "side_module_trampoline_a");
  assert(side_module_trampolinea != NULL);
  int res = side_module_trampolinea(f);
  printf("okay %d\n", res);
}


EMSCRIPTEN_KEEPALIVE void not_promising() {
  helper(test_sync_wrapper);
}

EM_JS(void, js_trampoline, (), {
  _not_promising();
})

int main() {
  printf("Suspending test\n");
  helper(test_suspending_wrapper);
  printf("Non suspending test\n");
  js_trampoline();
  printf("done\n");
  return 0;
}

