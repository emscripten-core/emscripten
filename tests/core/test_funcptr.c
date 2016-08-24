#include <stdio.h>
int calc1() { return 26; }
int calc2() { return 90; }
typedef int (*fp_t)();

fp_t globally1 = calc1;
fp_t globally2 = calc2;

int nothing(const char *str) { return 0; }

int main() {
  fp_t fp = calc1;
  void *vp = (void *)fp;
  fp_t fpb = (fp_t)vp;
  fp_t fp2 = calc2;
  void *vp2 = (void *)fp2;
  fp_t fpb2 = (fp_t)vp2;
  printf("*%d,%d,%d,%d,%d,%d*\n", fp(), fpb(), fp2(), fpb2(), globally1(),
         globally2());

  fp_t t = calc1;
  printf("*%d,%d", t == calc1, t == calc2);
  t = calc2;
  printf(",%d,%d*\n", t == calc1, t == calc2);

  int (*other)(const char * str);
  other = nothing;
  other("*hello!*");
  other = puts;
  other("*goodbye!*");

  return 0;
}
