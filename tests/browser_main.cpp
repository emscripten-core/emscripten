#include <assert.h>
#include <stdio.h>
#include <dlfcn.h>
#include <emscripten.h>

typedef void (*voidfunc)();
typedef int (*intfunc)();

void *lib_handle;
voidfunc onefunc;
intfunc twofunc;

void next(const char *x) {
  lib_handle = dlopen("themodule.js", RTLD_NOW);
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
  REPORT_RESULT();
}

int main() {
  emscripten_async_wget("module.js", "themodule.js", next, NULL);
  
  return 0;
}

