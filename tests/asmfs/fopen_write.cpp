#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <emscripten/emscripten.h>

void create_file()
{
  FILE *file = fopen("hello_file.txt", "wb");
  assert(file);
  const char *data = "Hello data!";
  fwrite(data, 1, strlen(data), file);
  fclose(file);
}

void read_file()
{
  FILE *file = fopen("hello_file.txt", "rb");
  char buffer[128] = {};
  size_t read = fread(buffer, 1, sizeof(buffer), file);
  printf("read %u bytes. Result: %s\n", (unsigned int)read, buffer);
  assert(read == strlen("Hello data!"));
  assert(!strcmp(buffer, "Hello data!"));
}

int main()
{
  create_file();
  read_file();

#ifdef REPORT_RESULT
  REPORT_RESULT(0);
#endif
}
