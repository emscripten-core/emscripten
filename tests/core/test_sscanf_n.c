#include <stdio.h>
int main() {
  const char *line = "version 1.0";
  int i, l, lineno;
  char word[80];
  if (sscanf(line, "%s%n", word, &l) != 1) {
    printf("Header format error, line %d\n", lineno);
  }
  printf("[DEBUG] word 1: %s, l: %d\n", word, l);

  int x = sscanf("one %n two", "%s %n", word, &l);
  printf("%d,%s,%d\n", x, word, l);
  {
    int a, b, c, count;
    count = sscanf("12345 6789", "%d %n%d", &a, &b, &c);
    printf("%i %i %i %i\n", count, a, b, c);
  }
  return 0;
}
