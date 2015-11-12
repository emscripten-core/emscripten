#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

void recurse(int x);

void act(volatile int *a) {
  printf("act     %d\n", *a);
  int b = (int)(intptr_t)(alloca(*a));
  if (b < *a) *a--;
  recurse(*a);
}

void recurse(int x) {
  printf("recurse %d\n", x);
  volatile int a = x;
  if (x*x < x) {
    act(&a);
    if (a < x) x = a;
    x--;
    return;
  }
  if (x > 0) recurse(x-1);
}

int main() {
  recurse(1000*1000);
}

