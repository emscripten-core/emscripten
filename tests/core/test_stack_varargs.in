// We should not blow up the stack with numerous varargs
#include <stdio.h>
#include <stdlib.h>

void func(int i) {
  printf(
      "%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,"
      "%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d\n",
      i, i, i, i, i, i, i, i, i, i, i, i, i, i, i, i, i, i, i, i, i, i, i, i, i,
      i, i, i, i, i, i, i, i, i, i, i, i, i, i);
}
int main() {
  for (int i = 0; i < 1024; i++) func(i);
  printf("ok!\n");
  return 0;
}
