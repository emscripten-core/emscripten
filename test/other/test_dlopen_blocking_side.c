#include <stdio.h>
#include <stdlib.h>

int foo = 42;

void my_atexit() {
  puts("side module atexit");
}

__attribute__((constructor)) static void ctor() {
  puts("side module ctor");
  atexit(my_atexit);
}
