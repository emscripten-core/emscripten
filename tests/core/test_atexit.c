#include <stdio.h>
#include <stdlib.h>

static void cleanA() { printf("A"); }
static void cleanB() { printf("B"); }

int main() {
  atexit(cleanA);
  atexit(cleanB);
  return 0;
}
