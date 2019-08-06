#include <alloca.h>
#include <string.h>
#include <stdlib.h>

int f(int *ptr) {
  for (int i = 0; i < 16384; ++i)
    ptr[i] = rand();
  return ptr[16383];
}

int main() {
  return f((int*) alloca(65536));
}
