#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

extern int async_func();

int main() {
  int result = async_func();
  assert(result == 42);
  printf("done\n");
  exit(0);
}
