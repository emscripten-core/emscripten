#include <stdio.h>
#include <string.h>
#include <assert.h>

#include<emscripten.h>

int value = 0;

extern "C" {
  void set(int x) {
    printf("set! %d\n", x);
    value = x;
  }
}

void load2() {
  printf("load2\n");

  char buffer[10];
  memset(buffer, 0, 10);
  FILE *f = fopen("file1.txt", "r");
  fread(buffer, 1, 5, f);
  fclose(f);
  assert(strcmp(buffer, "first") == 0);

  memset(buffer, 0, 10);
  f = fopen("file2.txt", "r");
  fread(buffer, 1, 6, f);
  fclose(f);
  assert(strcmp(buffer, "second") == 0);

  int result = 1;
  REPORT_RESULT();
}
void error2() {
  printf("fail2\n");
}

void load1() {
  printf("load1\n");
  assert(value == 456);
  emscripten_async_load_script("script2.js", load2, error2);
}
void error1() {
  printf("fail1\n");
}

int main() {
  emscripten_async_load_script("script1.js", load1, error1);

  return 1;
}

