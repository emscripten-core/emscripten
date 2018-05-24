#include <stdio.h>
#include <fcntl.h>
typedef int (*fp_t)(int, int, ...);
int main(int argc, char **argv) {
  fp_t fp = &fcntl;
  if (argc == 1337) fp = (fp_t) & main;
  (*fp)(0, 10);
  (*fp)(0, 10, 5);
  printf("waka\n");
  return 0;
}
