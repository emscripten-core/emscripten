#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <assert.h>

int main() {
  uint8_t *ptr1 = (uint8_t *)malloc(1024*1024);
  assert(ptr1);
  uint8_t *ptr2 = (uint8_t *)malloc(3u*1024*1024*1024);
  assert(ptr2);
  uint8_t *ptr3 = (uint8_t *)malloc(1024*1024);
  assert(ptr3);
  free(ptr1);
  free(ptr3);
  free(ptr2);
  printf("OK\n");
  return 0;
}
