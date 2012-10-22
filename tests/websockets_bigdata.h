
#include <stdlib.h>

#define DATA_SIZE 1250
// 1500 fails

char *generateData() {
  char *ret = malloc(65536*2);
  char *curr = ret;
  for (int i = 0; i < 256; i++) {
    for (int j = 0; j < 256; j++) {
      *curr = i;
      curr++;
      *curr = j;
      curr++;
    }
  }
  return ret;
}

