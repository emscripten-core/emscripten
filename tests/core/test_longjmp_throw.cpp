#include <stdio.h>
#include <setjmp.h>

static jmp_buf buf;
volatile int x = 0;

void second(void) {
  printf("second\n");
  if (x == 17) throw 5;
  else longjmp(buf, -1);
}

void first(void) {
  printf("first\n");
  longjmp(buf, 1);
}

int main() {
  int jmpval = setjmp(buf);
  if (!jmpval) {
    x++;
    first();
    printf("skipped\n");
  } else if (jmpval == 1) {
    printf("result: %d %d\n", x, jmpval);
    x++;
    try {
      second();
    } catch(int a) {
      x--;
      second();
    }
  } else if (jmpval == -1) {
    printf("result: %d %d\n", x, jmpval);
  }

  return 0;
}
