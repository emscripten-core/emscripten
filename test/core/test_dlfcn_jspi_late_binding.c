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

// side_a.so calls side_module_trampoline_b, which is defined in side_b.so.
// Since side_b.so is loaded after side_a.so, the call from a to b goes
// through a late-binding stub.
void helper(F f) {
  void* handle = dlopen("side_a.so", RTLD_NOW|RTLD_GLOBAL);
  assert(handle != NULL);
  G side_module_trampoline_a = dlsym(handle, "side_module_trampoline_a");
  assert(side_module_trampoline_a != NULL);
  int res = side_module_trampoline_a(f);
  printf("okay %d\n", res);
}

// Called from JS via a non-promising export: the stub must also work when
// there is no suspender available.
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
