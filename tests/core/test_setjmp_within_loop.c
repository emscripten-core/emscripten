#include <setjmp.h>
#include <stdio.h>

static jmp_buf buf;

void a() { printf("a\n"); }
void b() { printf("b\n"); }
void c() { printf("c\n"); }
int d() {
  static int first = 1;
  if (first) {
    first = 0;
    printf("d\n");
    return 1;
  }
  return 0;
}

int n = 3;

int main() {
  while (n-- > 0) {
    a();
    if (setjmp(buf))
      b();
    else
      c();
  }
  if (d())
    longjmp(buf, 3);
  return 0;
}
