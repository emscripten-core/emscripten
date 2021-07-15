// Copyright 2013 The Emscripten Authors.  All rights reserved.
// Emscripten is available under two separate licenses, the MIT license and the
// University of Illinois/NCSA Open Source License.  Both these licenses can be
// found in the LICENSE file.

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <dlfcn.h>
#include <emscripten.h>

typedef void (*voidfunc)();
typedef int (*intfunc)();

void *lib_handle;
voidfunc onefunc;
intfunc twofunc;

void next(const char *x) {
  lib_handle = dlopen("thelib.wasm", RTLD_NOW);
  assert(lib_handle != NULL);

  onefunc = (voidfunc)dlsym(lib_handle, "one");
  twofunc = (intfunc)dlsym(lib_handle, "two");
  assert(onefunc && twofunc);

  assert(twofunc() == 0);
  onefunc();
  assert(twofunc() == 1);
  onefunc();
  onefunc();
  assert(twofunc() == 3);
  onefunc();
  onefunc();
  onefunc();
  onefunc();
  assert(twofunc() == 7);
  onefunc();
  int result = twofunc();
  assert(result == 8);
  exit(0);
}

int main() {
  emscripten_async_wget("lib.wasm", "thelib.wasm", next, NULL);
  printf("returning from main\n");
  return 0;
}

