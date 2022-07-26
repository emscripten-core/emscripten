#include <string.h>

volatile int ten = 10;

int main(void) {
  char *x = new char[10];
  memset(x, 0, 10);
  return x[ten];
}
