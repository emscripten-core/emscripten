#include <stdio.h>

__attribute__((constructor)) void init(void) {
  printf("init\n");
}
