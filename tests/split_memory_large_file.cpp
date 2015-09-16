#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include <stdint.h>

#include <emscripten.h>

#define TOTAL_SIZE (4*1024*1024)

int main() {
  unsigned char temp;

  printf("open file\n");
  FILE *f = fopen("huge.dat", "r");
  assert(f);
  unsigned i;
  for (i = 0; i < TOTAL_SIZE; i++) {
    int ret = fseek(f, i, SEEK_SET);
    assert(ret == 0);
    int num = fread(&temp, 1, 1, f);
    assert(num == 1);
    unsigned char expected = (i*i)&255;
    if (temp != expected) {
      printf("%d: see %d != %d was expected :(\n", i, temp, expected);
      abort();
    }
    if ((i & (1024*1024-1)) == 0) printf("%d of %d ..\n", i, TOTAL_SIZE);
  }
  printf("%d all ok.\n", i);
  int result = 1;
  REPORT_RESULT();
}

