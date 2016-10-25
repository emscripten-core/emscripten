#include <stdio.h>
#include <assert.h>
#include <sys/timeb.h>

int main() {
  timeb tb;
  tb.timezone = 1;
  printf("*%d\n", ftime(&tb));
  assert(tb.time > 10000);
  assert(tb.timezone == 0);
  assert(tb.dstflag == 0);
  return 0;
}
