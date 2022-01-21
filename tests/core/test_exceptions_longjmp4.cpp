#include <stdio.h>
#include <setjmp.h>

static jmp_buf buf;

void foo() {
  throw 3;
}

int main() {
  int jmpval = setjmp(buf);
  if (jmpval != 0) {
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
