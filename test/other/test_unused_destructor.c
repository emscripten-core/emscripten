#include <stdio.h>
#include <stdlib.h>

__attribute((destructor)) void dtor() {
  printf("hello from dtor");
  abort();
}

int main() {
  printf("in main\n");
  return 0;
}
