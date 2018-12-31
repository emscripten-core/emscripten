#include <stdio.h>
int test(int i) {
  int x = 10;
  int ret = reinterpret_cast<long>(&x);  // both for the number, and forces x to not be nativized
  if (i > 0) {
    if ((i % 2001) != 1500)
      return test(i - 1);
    else
      return test(i - 1) + test(i - 2);
  }
  return ret;
}
int main(int argc, char **argv) {
  // We should get the same value for the first and last - stack has unwound
  int x1 = test(argc - 2);
  int x2 = test(100);
  int x3 = test((argc - 2) / 4);
  printf("*%d,%d*\n", x3 - x1, x2 != x1);
  return 0;
}
