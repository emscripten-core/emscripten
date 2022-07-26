#include <stdio.h>
#include <setjmp.h>

static jmp_buf buf;

void foo() {
  longjmp(buf, 1);
}

void test() {
  try {
    foo();
  } catch (...) {
  }
}

int main() {
  int jmpval = setjmp(buf);
  if (jmpval != 0) {
    printf("setjmp returned for second time\n");
    return 0;
  }
  test();
  return 0;
}
