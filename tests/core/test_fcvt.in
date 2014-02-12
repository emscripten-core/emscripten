/* This example borrowed from MSDN documentation */#include <stdlib.h>
#include <stdio.h>

int main() {
  int decimal, sign;
  char *buffer;
  double source = 3.1415926535;

  buffer = fcvt(source, 7, &decimal, &sign);
  printf("source: %2.10f   buffer: '%s'   decimal: %d   sign: %d\n", source,
         buffer, decimal, sign);
}
