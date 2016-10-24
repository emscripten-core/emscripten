#include <stdio.h>
int main () {
  *(volatile char *)0 = 0;
  return *(volatile char *)0;
}
