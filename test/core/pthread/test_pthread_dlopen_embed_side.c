#include <assert.h>
#include <stdio.h>
#include <string.h>

void read_embedded_file() {
  FILE* f = fopen("embedded.txt", "r");
  assert(f);
  char buf[64] = {0};
  size_t n = fread(buf, 1, sizeof(buf) - 1, f);
  assert(n > 0);
  assert(strcmp(buf, "hello from embedded file\n") == 0);
  fclose(f);
  printf("side: %s", buf);
}
