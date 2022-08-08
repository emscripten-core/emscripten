#include <stdio.h>

int get_value();

int main(void) {
  try {
    // throws 42
    get_value();
  } catch (int i) {
    printf("except: %d\n", i);
  }
  return 0;
}
