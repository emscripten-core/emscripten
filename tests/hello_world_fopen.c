#include<stdio.h>

int main() {
  FILE* f = fopen("/dev/stdout", "w");
  fprintf(f, "hello, world!\n");
  fclose(f);
  return 0;
}

