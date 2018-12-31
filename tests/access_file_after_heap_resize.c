#include <stdio.h>
#include <stdlib.h>

int main() {
  FILE *fp;
  int c;

  malloc(20000000);  // Enlarge memory

  fp = fopen("test.txt", "r");
  int nChars = 0;
  while ((c = fgetc(fp)) != EOF)
  {
    putchar(c);
    ++nChars;
  }
#ifdef REPORT_RESULT
  REPORT_RESULT(nChars);
#endif
}
