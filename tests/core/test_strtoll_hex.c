#include <stdio.h>
#include <stdlib.h>

int main() {
  const char *STRING = "0x4 -0x3A +0xDEADBEEF";
  char *end_char;

  // undefined base
  long long int l1 = strtoll(STRING, &end_char, 0);
  long long int l2 = strtoll(end_char, &end_char, 0);
  long long int l3 = strtoll(end_char, NULL, 0);

  // defined base
  long long int l4 = strtoll(STRING, &end_char, 16);
  long long int l5 = strtoll(end_char, &end_char, 16);
  long long int l6 = strtoll(end_char, NULL, 16);

  printf("%d%d%d%d%d%d\n", l1 == 0x4, l2 == -0x3a, l3 == 0xdeadbeef, l4 == 0x4,
         l5 == -0x3a, l6 == 0xdeadbeef);
  return 0;
}
