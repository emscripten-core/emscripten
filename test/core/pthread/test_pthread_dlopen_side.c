#include <stdio.h>
#include <stdlib.h>

typedef int (*myfunc_type)();

static int mydata[10] = { 44 };

static void dtor() {
  puts("side module atexit ..");
}

__attribute__((constructor)) static void ctor() {
  puts("side module ctor");
  atexit(dtor);
}

static int myfunc() {
  return 43;
}

// Exposing the address of `mydata` in this way forces it to
// be present in the .data segment with its address calculated
// relative the `__memory_base` at which the DSO is loaded.
int* side_data_address() {
  return mydata;
}

// Exposing the address of `mydata` in this way forces it to
// be present in static table region on the DSO and its
// address will be calculated relative to the `__table_base`
// at which the DSO is loaded.
myfunc_type side_func_address() {
  return &myfunc;
}
