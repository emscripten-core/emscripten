#include <stdio.h>
#include <errno.h>
#include <unistd.h>

int main() {
  printf("%ld, ", sysconf(_SC_PHYS_PAGES));
  printf("errno: %d\n", errno);
  return 0;
}
