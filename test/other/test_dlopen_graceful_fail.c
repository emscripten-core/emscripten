// Emscripten's stub implementation of dlopen() hard aborts execution.
// Some users want to instead have dlopen() perform a graceful "return 0" failure
// where they handle the dlopen failures in code, so they want to override
// Emscripten's built-in JS library functions.

// This test verifies that the ability to override dlopen() to not hard abort
// will not regress.
#include <dlfcn.h>
#include <stdio.h>

int main()
{
  printf("Attempting to dlopen\n");
  void *dll = dlopen("nonexistent.dll", 0);
  printf("dlopen completed: ptr %p. error: %s\n", dll, dlerror());
  dlclose(0);
  printf("dlclose(0) completed.\n");
  if (!dll) printf("dlopen failed gracefully!\n");
  return 0;
}
