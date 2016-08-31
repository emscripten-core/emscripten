#include <stdio.h>
int main() {
  int size = snprintf(NULL, 0, "%s %d %.2f\n", "me and myself", 25, 1.345);
  char buf[size];
  snprintf(buf, size, "%s %d %.2f\n", "me and myself", 25, 1.345);
  printf("%d : %s\n", size, buf);
  char *buff = NULL;
  asprintf(&buff, "%d waka %d\n", 21, 95);
  puts(buff);
  return 0;
}
