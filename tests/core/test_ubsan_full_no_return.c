#include <stdlib.h>

int test(int x) {
  if (x) return 1;
}

int main(int argc, char **argv) {
  putenv("UBSAN_OPTIONS=color=never");

  test(0);
  return 0;
}
