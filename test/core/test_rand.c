#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>
#include <assert.h>

int main() {
  // We need RAND_MAX to be a bitmask (power of 2 minus 1).  This assertion will
  // error if RAND_MAX ever changes, so we don't miss that.
  assert(RAND_MAX == 0x7fffffff);

  srand(0xdeadbeef);
  for (int i = 0; i < 10; ++i) {
    printf("%d\n", rand());
  }

  unsigned int seed = 0xdeadbeef;
  for (int i = 0; i < 10; ++i) {
    printf("%d\n", rand_r(&seed));
  }

  bool haveEvenAndOdd = true;
  for (int i = 1; i <= 30; ++i) {
    int mask = 1 << i;
    if (mask > RAND_MAX) break;
    bool haveEven = false;
    bool haveOdd = false;
    for (int j = 0; j < 1000 && (!haveEven || !haveOdd); ++j) {
      if ((rand() & mask) == 0) {
        haveEven = true;
      } else {
        haveOdd = true;
      }
    }
    haveEvenAndOdd = haveEvenAndOdd && haveEven && haveOdd;
  }

  if (haveEvenAndOdd) {
    printf("Have even and odd!\n");
  }

  return 0;
}
