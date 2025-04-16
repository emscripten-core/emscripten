/*
 * Copyright 2022 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */

#include <stdio.h>
#include <sys/mman.h>
#include <string.h>
#include <assert.h>

int main() {
  FILE *f = fopen("lazy.txt", "r");
  assert(f);
  char *data = (char*)mmap(NULL, 1024, PROT_READ, MAP_PRIVATE, fileno(f), 0);
  assert(data);
  printf("len: %lu\n", strlen(data));
  printf("data: %s\n", data);
  assert(strcmp(data, "hello world") == 0);
  return 0;
}
