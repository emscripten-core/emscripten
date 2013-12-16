#include <stdio.h>
int main(void) {
  const void *addrs[2] = {&&FOO, &&BAR};

  // confuse the optimizer so it doesn't hardcode the jump and avoid generating
  // an |indirectbr| instruction
  int which = 0;
  for (int x = 0; x < 1000; x++) which = (which + x * x) % 7;
  which = (which % 2) + 1;

  goto *addrs[which];

FOO:
  printf("bad\n");
  return 0;
BAR:
  printf("good\n");
  const void *addr = &&FOO;
  goto *addr;
}
