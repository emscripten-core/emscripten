#include <stdio.h>
#include <stdlib.h>

int main() {
  const char *STRING = "4 -38 +4711";
  char *end_char;

  // undefined base
  long l1 = strtol(STRING, &end_char, 0);
  long l2 = strtol(end_char, &end_char, 0);
  long l3 = strtol(end_char, NULL, 0);

  // defined base
  long l4 = strtol(STRING, &end_char, 10);
  long l5 = strtol(end_char, &end_char, 10);
  long l6 = strtol(end_char, NULL, 10);

  printf("%d%d%d%d%d%d\n", l1 == 4, l2 == -38, l3 == 4711, l4 == 4, l5 == -38,
         l6 == 4711);
  return 0;
}
