#include <sys/mman.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

int main()
{
  FILE *fd = fopen("test.txt", "w");
  fputs("12345678", fd);
  fclose(fd);

  fd = fopen("test.txt", "r");
  char *data = (char*)mmap(0, 32000000, PROT_READ, MAP_PRIVATE, fileno(fd), 0);
  assert(!strcmp(data, "12345678"));
  printf("%s\n", data);
  return 0;
}
