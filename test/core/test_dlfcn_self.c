/*
 * Copyright 2016 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */

#include <assert.h>
#include <stdio.h>
#include <dlfcn.h>
#include <emscripten/em_macros.h>

int EMSCRIPTEN_KEEPALIVE global = 123;

EMSCRIPTEN_KEEPALIVE void foo(int x) {
  printf("%d\n", x);
}

void repeatable() {
  void* self = dlopen(NULL, RTLD_LAZY);
  assert(self);

  int* global_ptr = (int*)dlsym(self, "global");
  assert(global_ptr);
  assert(*global_ptr = 123);

  void (*foo_ptr)(int) = (void (*)(int))dlsym(self, "foo");
  assert(foo_ptr);

  foo_ptr(*global_ptr);
  dlclose(self);
}

int main() {
  repeatable();
  repeatable();
  return 0;
}
