/*
 * Copyright 2016 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */

#include <stdio.h>

double get() {
  double ret = 0;
  __asm __volatile__("Math.abs(-12/3.3)" : "=r"(ret));  // write to a variable
  asm("#comment1");
  asm volatile("#comment2");
  asm volatile(
      "#comment3\n"
      "#comment4\n");
  return ret;
}

int main() {
  asm("out('Inline JS is very cool')");
  printf("%.2f\n", get());

  // Test that passing multiple input and output variables works.
  int src1 = 1, src2 = 2, src3 = 3;
  int dst1 = 0, dst2 = 0, dst3 = 0;
  // TODO asm("out(%3); out(%4); out(%5); %0 = %3; %1
  // = %4; %2 = %5;" : "=r"(dst1),"=r"(dst2),"=r"(dst3):
  // "r"(src1),"r"(src2),"r"(src3));
  // TODO printf("%d\n%d\n%d\n", dst1, dst2, dst3);

  return 0;
}
