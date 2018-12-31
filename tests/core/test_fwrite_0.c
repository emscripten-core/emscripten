#include <stdio.h>
#include <stdlib.h>

int main() {
  FILE *fh;

  fh = fopen("a.txt", "wb");
  if (!fh) exit(1);
  fclose(fh);

  fh = fopen("a.txt", "rb");
  if (!fh) exit(1);

  char data[] = "foobar";
  size_t written = fwrite(data, 1, sizeof(data), fh);

  printf("written=%zu\n", written);
}
