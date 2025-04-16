#include <stdio.h>
#include <string.h>

int main() {
  FILE *handle = fopen("test", "wb");
  fputs("hello from file!", handle);
  fclose(handle);
  handle = fopen("test", "r");
  char str[256] = {};
  fgets(str, 255, handle);
  printf("%s\n", str);
  return strcmp(str, "hello from file!");
}
