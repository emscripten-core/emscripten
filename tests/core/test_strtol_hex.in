#include <stdio.h>
#include <stdlib.h>

int main() {
  const char *STRING = "0x4 -0x3A +0xDEAD";
  char *end_char;

  // undefined base
  long l1 = strtol(STRING, &end_char, 0);
  long l2 = strtol(end_char, &end_char, 0);
  long l3 = strtol(end_char, NULL, 0);

  // defined base
  long l4 = strtol(STRING, &end_char, 16);
  long l5 = strtol(end_char, &end_char, 16);
  long l6 = strtol(end_char, NULL, 16);

  printf("%d%d%d%d%d%d\n", l1 == 0x4, l2 == -0x3a, l3 == 0xdead, l4 == 0x4,
         l5 == -0x3a, l6 == 0xdead);
  return 0;
}
