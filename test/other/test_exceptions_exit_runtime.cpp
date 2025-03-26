#include <stdio.h>
#include <stdlib.h>

void foo() {
  printf("foo\n");
  exit(0);
}

int main() {
  printf("main\n");
  try {
    // `foo` calls `exit()` which triggers the runtime to exit and then
    // the stack to unwind.
    // This test verifies that we don't error during the unwinding due
    // to the runtime having already exited.
    foo();
  } catch(...) {
    printf("caught\n");
  }
}
