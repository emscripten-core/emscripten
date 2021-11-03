#include <emscripten/emscripten.h>
#include <assert.h>
#include <stdlib.h>
#include <dlfcn.h>
#include <stdio.h>

int mydata = 0;

void onsuccess(void *user_data, void *handle) {
  assert(user_data == &mydata);
  printf("onsuccess\n");
  int* foo = (int*)dlsym(handle, "foo");
  assert(foo);
  printf("foo = %d\n", *foo);
  assert(*foo == 42);
  exit(0);
}

void onerror(void *user_data) {
  assert(user_data == &mydata);
  printf("onerror %s\n", dlerror());
}

int main() {
  emscripten_dlopen("libside.so", RTLD_NOW, &mydata, onsuccess, onerror);
  printf("returning from main\n");
  return 99;
}
