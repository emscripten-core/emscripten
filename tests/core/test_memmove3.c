#include <stdio.h>
#include <string.h>
int main() {
  char str[] = "memmove can be vvery useful....!";
  memmove(str + 15, str + 16, 17);
  puts(str);
  return 0;
}
