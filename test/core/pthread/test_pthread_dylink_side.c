#include <stdio.h>
#include "test_pthread_dylink.h"

int foo_side = 55;

int bar_side() {
  return 42;
}

void print_addresses_side() {
  printf("side_module: &foo_main %p -> %d\n", &foo_main, foo_main);
  printf("side_module: &foo_side %p -> %d\n", &foo_side, foo_side);
  printf("side_module: &bar_main %p\n", &bar_main);
  printf("side_module: &bar_side %p\n", &bar_side);
}
