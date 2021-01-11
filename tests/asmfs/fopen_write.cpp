// Copyright 2016 The Emscripten Authors.  All rights reserved.
// Emscripten is available under two separate licenses, the MIT license and the
// University of Illinois/NCSA Open Source License.  Both these licenses can be
// found in the LICENSE file.

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <emscripten/emscripten.h>

void create_file()
{
  FILE *file = fopen("hello_file.txt", "wb");
  assert(file);
  const char *data = "Hello world";
  const char *data2 = "da";
  const char *data3 = "ta!";
  fwrite(data, 1, strlen(data), file);
  fseek(file, 8, SEEK_SET);
  fwrite(data3, 1, strlen(data3), file);
  fseek(file, 6, SEEK_SET);
  fwrite(data2, 1, strlen(data2), file);
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
  return 0;
}
