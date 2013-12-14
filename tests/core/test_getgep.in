#include <stdio.h>
struct {
  int y[10];
  int z[10];
} commonblock;

int main() {
  for (int i = 0; i < 10; ++i) {
    commonblock.y[i] = 1;
    commonblock.z[i] = 2;
  }
  printf("*%d %d*\n", commonblock.y[0], commonblock.z[0]);
  return 0;
}
