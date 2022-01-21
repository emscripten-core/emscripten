#include <stdio.h>
#include <setjmp.h>

static jmp_buf buf;

void foo() {
  throw 3;
}

int main() {
  int jmpval = setjmp(buf);
  if (jmpval != 0) {
    // This is not reached, because foo() doesn't longjmp. This test checks
    // compilation of a setjmp call with a nested try.
    printf("setjmp returned for the second time\n");
    return 0;
  }
  try {
    foo();
    try {
      foo();
    } catch (int n) {
      printf("inner catch: caught %d\n", n);
    }
  } catch (int n) {
    printf("outer catch: caught %d\n", n);
  }
  return 0;
}
