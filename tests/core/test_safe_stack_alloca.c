#include <alloca.h>
#include <string.h>
#include <stdlib.h>

#define STACK_SIZE 65536

int f(int *ptr) {
  for (int i = 0; i < (STACK_SIZE/sizeof(int) + 1); ++i)
    ptr[i] = rand();
  return ptr[0];
}

int main() {
  return f((int*) alloca(STACK_SIZE+1));
}
