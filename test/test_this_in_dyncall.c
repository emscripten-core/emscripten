#include <stdio.h>

void foo(int *x) {
  printf("%d\n", *x);
}

void test(void(*fnptr)(int *), int *addr);

int main() {
  int val = 42;
  test(&foo, &val);
}
