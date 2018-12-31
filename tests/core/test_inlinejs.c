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
  asm("Module.print('Inline JS is very cool')");
  printf("%.2f\n", get());

  // Test that passing multiple input and output variables works.
  int src1 = 1, src2 = 2, src3 = 3;
  int dst1 = 0, dst2 = 0, dst3 = 0;
  // TODO asm("Module.print(%3); Module.print(%4); Module.print(%5); %0 = %3; %1
  // = %4; %2 = %5;" : "=r"(dst1),"=r"(dst2),"=r"(dst3):
  // "r"(src1),"r"(src2),"r"(src3));
  // TODO printf("%d\n%d\n%d\n", dst1, dst2, dst3);

  return 0;
}
