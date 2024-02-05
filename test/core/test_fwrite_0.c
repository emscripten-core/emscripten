/*
 * Copyright 2016 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */

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
