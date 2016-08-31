#include <stdio.h>

struct malloc_params {
  size_t magic, page_size;
};

malloc_params mparams;

#define SIZE_T_ONE ((size_t)1)
#define page_align(S) \
  (((S) + (mparams.page_size - SIZE_T_ONE)) & ~(mparams.page_size - SIZE_T_ONE))

int main() {
  mparams.page_size = 4096;
  printf("*%d,%d,%d,%d*\n", mparams.page_size, page_align(1000),
         page_align(6000), page_align(66474));
  return 0;
}
