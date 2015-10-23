#include <stdio.h>
#include <string.h>

int main() {
  const char *hello = "hello world";
  int result = 1;
  putc('g', stdout);
  putc('o', stdout);
  putc('!', stdout);
  if (strchr(hello, 'l') != hello + 2) result = 0;
  if (strchr(hello, 'w') != hello + 6) result = 0;
  putc('d', stdout);
  putc('o', stdout);
  putc('n', stdout);
  putc('e', stdout);
  REPORT_RESULT();
  return 0;
}

