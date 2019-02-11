#include <stdio.h>
int main() {
  int byte_count = 64;
  char data[64];
  FILE *fp;
  fp = fopen("/dev/random", "r");
  fread(&data, 1, byte_count, fp);
  fclose(fp);
  REPORT_RESULT(0);
  return 0;
}

