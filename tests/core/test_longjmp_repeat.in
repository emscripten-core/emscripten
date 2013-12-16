#include <stdio.h>
#include <setjmp.h>

static jmp_buf buf;

int main() {
  volatile int x = 0;
  printf("setjmp:%d\n", setjmp(buf));
  x++;
  printf("x:%d\n", x);
  if (x < 4) longjmp(buf, x * 2);
  return 0;
}
