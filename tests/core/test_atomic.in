#include <stdio.h>
int main() {
  int x = 10;
  int y = __sync_add_and_fetch(&x, 5);
  printf("*%d,%d*\n", x, y);
  x = 10;
  y = __sync_fetch_and_add(&x, 5);
  printf("*%d,%d*\n", x, y);
  x = 10;
  y = __sync_lock_test_and_set(&x, 6);
  printf("*%d,%d*\n", x, y);
  x = 10;
  y = __sync_bool_compare_and_swap(&x, 9, 7);
  printf("*%d,%d*\n", x, y);
  y = __sync_bool_compare_and_swap(&x, 10, 7);
  printf("*%d,%d*\n", x, y);
  return 0;
}
