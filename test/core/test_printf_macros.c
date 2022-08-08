#include <stdio.h>
#include <inttypes.h>

int main() {
  intptr_t i = 3;
  uintptr_t u = 4;
  printf("%" PRIiPTR "\n", i);
  printf("%" PRIdPTR "\n", i);
  printf("%" PRIxPTR "\n", i);
  printf("%" PRIuPTR "\n", u);
  // TODO(sbc): test the rest of these macros.
  return 0;
}
