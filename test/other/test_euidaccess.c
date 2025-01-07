#define _GNU_SOURCE

#include <unistd.h>
#include <stdio.h>

int main() {
  printf("eaccess:%d\n", eaccess(".", F_OK));
  printf("euidaccess:%d\n", euidaccess(".", F_OK));
  return 0;
}
