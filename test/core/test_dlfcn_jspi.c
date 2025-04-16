// Copyright 2025 The Emscripten Authors.  All rights reserved.
// Emscripten is available under two separate licenses, the MIT license and the
// University of Illinois/NCSA Open Source License.  Both these licenses can be
// found in the LICENSE file.

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
  void* handle = dlopen("side.so", RTLD_NOW|RTLD_GLOBAL);
  F side_module_trampoline = dlsym(handle, "side_module_trampoline");
  int res = side_module_trampoline();
  printf("done %d\n", res);
  return 0;
}
