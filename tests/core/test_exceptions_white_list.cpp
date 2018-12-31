#include <stdio.h>

void thrower() {
  printf("infunc...");
  throw(99);
  printf("FAIL");
}

void somefunction() {
  try {
    thrower();
  }
  catch (...) {
    printf("done!*\n");
  }
}

int main() {
  somefunction();
  return 0;
}
