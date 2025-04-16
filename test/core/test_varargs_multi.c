/*
 * Copyright 2016 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */

#include <stdarg.h>
#include <stdio.h>

void print_vararg(int n, va_list v) {
  for (int i = 0; i < n; ++i) {
    if (i > 0) {
      printf(", ");
    }
    printf("%d", va_arg(v, int));
  }
  printf("\n");
}

void multi_vararg(int n, ...) {
  va_list v;

  printf("part1:\n");
  va_start(v, n);
  print_vararg(n, v);
  va_end(v);

  printf("part2:\n");
  va_start(v, n);
  print_vararg(n, v);
  printf("part3:\n");
  print_vararg(n, v); // the value of v is undefined for this call
  va_end(v);

}

int main() {
  multi_vararg(5,
               8, 6, 4, 2, 0,
               1, 3, 5, 7, 9,
               4, 5, 6, 7, 8
               );
  return 0;
}
