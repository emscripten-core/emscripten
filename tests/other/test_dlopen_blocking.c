#include <assert.h>
#include <stdlib.h>
#include <dlfcn.h>
#include <stdio.h>

int main() {
  void* handle = dlopen("libside.so", RTLD_NOW);
  printf("handle: %p\n", handle);
  if (!handle) {
    printf("%s\n", dlerror());
    return 1;
  }
  int* foo = (int*)dlsym(handle, "foo");
  if (!foo) {
    printf("%s\n", dlerror());
    return 1;
  }
  printf("foo = %d\n", *foo);
  assert(*foo == 42);
  exit(0);
}
