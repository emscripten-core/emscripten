// Copyright 2025 The Emscripten Authors.  All rights reserved.
// Emscripten is available under two separate licenses, the MIT license and the
// University of Illinois/NCSA Open Source License.  Both these licenses can be
// found in the LICENSE file.

#include <assert.h>
#include <emscripten.h>
#include <stdio.h>
#include <dlfcn.h>

EM_ASYNC_JS(int, test, (), {
  console.log("sleeping");
  await new Promise(res => setTimeout(res, 0));
  console.log("slept");
  return 77;
});

int test_wrapper() {
  return test();
}

typedef int (*F)();

int main() {
  void* handle = dlopen("side_a.so", RTLD_NOW|RTLD_GLOBAL);
  assert(handle != NULL);
  F side_module_trampolinea = dlsym(handle, "side_module_trampoline_a");
  assert(side_module_trampolinea != NULL);
  int res = side_module_trampolinea();
  printf("done %d\n", res);
  return 0;
}
