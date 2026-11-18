// Copyright 2014 The Emscripten Authors.  All rights reserved.
// Emscripten is available under two separate licenses, the MIT license and the
// University of Illinois/NCSA Open Source License.  Both these licenses can be
// found in the LICENSE file.

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <emscripten/emscripten.h>
#include <emscripten/stack.h>

void recurse(unsigned long x);

void act(volatile unsigned long *a) {
  printf("act     %ld\n", *a);
  unsigned long b = (long)(intptr_t)(alloca(*a));
  if (b < *a) *a--;
  recurse(*a);
}

void recurse(volatile unsigned long x) {
  printf("recurse %ld sp=%#lx\n", x, emscripten_stack_get_current());
  volatile unsigned long a = x;
  volatile char buffer[1000*1000];
  buffer[x/2] = 0;
  buffer[(x-1)/2] = 0;
  EM_ASM({});
  // Read x first to avoid unsequenced access to a volatile.
  unsigned long y = x;
  if (y*y < y) {
    act(&a);
    if (a < x) x = a;
    x--;
  }
  y = x;
  x += buffer[y/2];
  if (x > 0) recurse(x-1);
}

int main() {
  recurse(1000*1000);
}
