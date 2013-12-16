#include <stdio.h>

int main(int argc, const char *argv[]) {
  char *a = new char[10];
  char *a0 = a + 0;
  char *a5 = a + 5;
  int *b = new int[10];
  int *b0 = b + 0;
  int *b5 = b + 5;
  int c = (int)b5 - (int)b0;  // Emscripten should warn!
  int d = (int)b5 - (int)b0;  // Emscripten should warn!
  printf("*%d*\n", (int)a5 - (int)a0);
  return 0;
}
