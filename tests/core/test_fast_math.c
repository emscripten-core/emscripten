#include <stdio.h>
#include <stdlib.h>

int main(int argc, char** argv) {
  char* endptr;
  --argc, ++argv;
  double total = 0.0;
  for (; argc; argc--, argv++) {
    total += strtod(*argv, &endptr);
  }
  printf("total: %g\n", total);
  return 0;
}
