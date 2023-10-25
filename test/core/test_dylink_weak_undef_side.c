#include <stdio.h>
#include <assert.h>

__attribute__((weak)) int bar();

void side() {
  printf("side &bar: %p\n", &bar);
  assert(&bar == NULL);
}
