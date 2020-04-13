#include <string.h>

volatile int ten = 10;

int main(void) {
  char x[10];
  memset(x, 0, sizeof x);
  return x[ten];
}
