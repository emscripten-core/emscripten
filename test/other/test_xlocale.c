#ifdef __cplusplus
// Test compatibility with libc++'s locale header
#include <locale>
#endif

#include <xlocale.h>
#include <stdio.h>

int main() {
  const char* input = "100001";
  locale_t l = newlocale(LC_ALL_MASK, "C", NULL);
  printf("strtoll_l: %lld\n", strtoll_l(input, NULL, 10, l));
  return 0;
}

