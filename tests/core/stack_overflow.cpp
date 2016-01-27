#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <emscripten.h>

void recurse(unsigned int x);

void act(volatile unsigned int *a) {
  printf("act     %d\n", *a);
  unsigned int b = (int)(intptr_t)(alloca(*a));
  if (b < *a) *a--;
  recurse(*a);
}

void recurse(volatile unsigned int x) {
  printf("recurse %d\n", x);
  volatile unsigned int a = x;
  volatile char buffer[1000*1000];
  buffer[x/2] = 0;
  buffer[(x-1)/2] = 0;
  EM_ASM({});
  if (x*x < x) {
    act(&a);
    if (a < x) x = a;
    x--;
  }
  x += buffer[x/2];
  if (x > 0) recurse(x-1);
}

int main() {
  recurse(1000*1000);
}

