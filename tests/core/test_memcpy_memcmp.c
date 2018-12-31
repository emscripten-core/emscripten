#include <stdio.h>
#include <string.h>
#include <assert.h>

#define MAXX 48
void reset(unsigned char *buffer) {
  for (int i = 0; i < MAXX; i++) buffer[i] = i + 1;
}
void dump(unsigned char *buffer) {
  for (int i = 0; i < MAXX - 1; i++) printf("%2d,", buffer[i]);
  printf("%d\n", buffer[MAXX - 1]);
}
int main() {
  unsigned char buffer[MAXX];
  for (int i = MAXX / 4; i < MAXX - MAXX / 4; i++) {
    for (int j = MAXX / 4; j < MAXX - MAXX / 4; j++) {
      for (int k = 1; k < MAXX / 4; k++) {
        if (i == j) continue;
        if (i < j && i + k > j) continue;
        if (j < i && j + k > i) continue;
        printf("[%d,%d,%d] ", i, j, k);
        reset(buffer);
        memcpy(buffer + i, buffer + j, k);
        dump(buffer);
        assert(memcmp(buffer + i, buffer + j, k) == 0);
        buffer[i + k / 2]++;
        if (buffer[i + k / 2] != 0) {
          assert(memcmp(buffer + i, buffer + j, k) > 0);
        } else {
          assert(memcmp(buffer + i, buffer + j, k) < 0);
        }
        buffer[i + k / 2]--;
        buffer[j + k / 2]++;
        if (buffer[j + k / 2] != 0) {
          assert(memcmp(buffer + i, buffer + j, k) < 0);
        } else {
          assert(memcmp(buffer + i, buffer + j, k) > 0);
        }
      }
    }
  }
  return 0;
}
