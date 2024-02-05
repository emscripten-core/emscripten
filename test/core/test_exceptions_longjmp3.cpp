#include <stdio.h>
#include <setjmp.h>

static jmp_buf buf;

void foo() {
  throw 3;
}

void test() {
  int jmpval = setjmp(buf);
  if (jmpval != 0) {
    printf("setjmp returned for 2nd time\n");
    return;
  }
  foo();
}

int main() {
  try {
    test();
  } catch (int n) {
    printf("caught: %d\n", n);
  }
  return 0;
}
