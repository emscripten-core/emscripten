#include <assert.h>
#include <stdlib.h>
#include <dlfcn.h>
#include <stdio.h>

int main() {
  void* handle = dlopen("libside.so", RTLD_NOW);
  if (!handle) {
    printf("dlopen failed: %s\n", dlerror());
    return 1;
  }
  printf("handle: %p\n", handle);
  int* foo = (int*)dlsym(handle, "foo");
  if (!foo) {
    printf("dlsym failed: %s\n", dlerror());
    return 1;
  }
  printf("foo = %d\n", *foo);
  assert(*foo == 42);
  exit(0);
}
