#include <stdio.h>
#include <string.h>
int main() {
  char str[] = "memmove can be very useful....!";
  memmove(str + 20, str + 15, 11);
  puts(str);
  return 0;
}
