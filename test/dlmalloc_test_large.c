#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main() {
  uintptr_t sbrk0 = (uintptr_t)sbrk(0);
  printf("%p ", malloc(0x10000000u));
  printf("%p ", malloc(0x80000000u));
  printf("%p ", malloc(0xF0000000u));
  printf("%d\n", (int)(sbrk0 == (uintptr_t)sbrk(0)));
}
