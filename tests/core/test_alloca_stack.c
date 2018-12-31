// We should not blow up the stack with numerous allocas
#include <stdio.h>
#include <stdlib.h>

int func(int i) {
  char *pc = (char *)alloca(100);
  *pc = i;
  (*pc)++;
  return (*pc) % 10;
}
int main() {
  int total = 0;
  for (int i = 0; i < 1024 * 1024; i++) total += func(i);
  printf("ok:%d*\n", total);
  return 0;
}
