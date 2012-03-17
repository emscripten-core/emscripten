#include <stdio.h>
int main() {
  FILE *file = fopen("tests/hello_world_file.txt", "rb");
  while (!feof(file)) {
    char c = fgetc(file);
    if (c != EOF) {
      putchar(c);
    }
  }
  fclose (file);
  return 0;
}

