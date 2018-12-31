#include <stdio.h>
#include <string.h>
#include <assert.h>
int main() {
  int sum = 0;
  char buffer[256];
  for (int i = 0; i < 10; i++) {
    for (int j = 0; j < 10; j++) {
      for (int k = 0; k < 35; k++) {
        for (int t = 0; t < 256; t++) buffer[t] = t;
        char *dest = buffer + i;
        char *src = buffer + j;
        if (dest == src) continue;
        // printf("%d, %d, %d\n", i, j, k);
        assert(memmove(dest, src, k) == dest);
        for (int t = 0; t < 256; t++) sum += buffer[t];
      }
    }
  }
  printf("final: %d.\n", sum);
  return 1;
}
