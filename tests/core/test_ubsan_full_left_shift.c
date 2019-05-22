#include <stdlib.h>

int main(void) {
  putenv("UBSAN_OPTIONS=color=never");

  int x = -1;
  x <<= 1;
  x = -1;
  x <<= 0;
  x = 16;
  x <<= (__SIZEOF_INT__ * __CHAR_BIT__ - 3);
}
