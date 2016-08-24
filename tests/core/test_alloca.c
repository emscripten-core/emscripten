#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

int main(int argc, char **argv) {
  char *pc, *pc2;
  assert(argc == 1);
  pc = (char *)alloca(4+argc);
  assert(((int)pc) % 4 == 0);
  pc2 = (char *)alloca(4+argc);
  assert(((int)pc2) % 4 == 0);
  printf("z:%d*%d*%d*\n", pc > 0, (int)pc, (int)pc2);
  return 0;
}
