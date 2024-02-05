#include <string.h>

volatile int ten = 10;

int main(void) {
  static char x[10];
  memset(x, 0, 10);
  return x[ten];
}
