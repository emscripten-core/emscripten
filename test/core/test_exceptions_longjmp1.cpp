#include <stdio.h>
#include <setjmp.h>

static jmp_buf buf;

void foo() {
  longjmp(buf, 1);
}

int main() {
  int jmpval = setjmp(buf);
  if (jmpval != 0) {
    printf("setjmp returned for the second time\n");
    return 0;
  }
  try {
    foo();
  } catch (...) {
  }
  return 0;
}
