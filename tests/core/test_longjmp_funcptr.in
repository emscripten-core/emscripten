#include <stdio.h>
#include <setjmp.h>

static jmp_buf buf;

void (*fp)() = NULL;

void second(void) {
  printf("second\n");  // prints
  longjmp(buf, 1);  // jumps back to where setjmp was called - making setjmp now
                    // return 1
}

void first(void) {
  fp();
  printf("first\n");  // does not print
}

int main(int argc, char **argv) {
  fp = argc == 200 ? NULL : second;

  volatile int x = 0;
  if (!setjmp(buf)) {
    x++;
    first();                  // when executed, setjmp returns 0
  } else {                    // when longjmp jumps back, setjmp returns 1
    printf("main: %d\n", x);  // prints
  }

  return 0;
}
