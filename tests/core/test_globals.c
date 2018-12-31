#include <stdio.h>

char cache[256], *next = cache;

int main() {
  cache[10] = 25;
  next[20] = 51;
  printf("*%d,%d*\n", next[10], cache[20]);
  return 0;
}
