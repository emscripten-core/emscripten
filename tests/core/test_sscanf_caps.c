#include "stdio.h"

int main() {
  unsigned int a;
  float e, f, g;
  sscanf("a 1.1 1.1 1.1", "%X %E %F %G", &a, &e, &f, &g);
  printf("%d %.1F %.1F %.1F\n", a, e, f, g);
}
