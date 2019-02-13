#include <stdio.h>

void thrower() {
  printf("infunc...");
  throw(99);
  printf("FAIL");
}

int main() {
  try {
    printf("*throw...");
    throw(1);
    printf("FAIL");
  } catch(...) {
    printf("caught!");
  }
  try {
    thrower();
  } catch(...) {
    printf("done!*\n");
  }
  return 0;
}
