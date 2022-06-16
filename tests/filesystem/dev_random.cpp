// Copyright 2019 The Emscripten Authors.  All rights reserved.
// Emscripten is available under two separate licenses, the MIT license and the
// University of Illinois/NCSA Open Source License.  Both these licenses can be
// found in the LICENSE file.

#include <assert.h>
#include <stdio.h>

int main() {
  int byte_count = 500;
  char data[500];
  FILE *fp;
  int nread;

  fp = fopen("/dev/random", "r");
  nread = fread(&data, 1, byte_count, fp);
  assert(nread == byte_count);
  fclose(fp);

  fp = fopen("/dev/urandom", "r");
  nread = fread(&data, 1, byte_count, fp);
  assert(nread == byte_count);
  fclose(fp);

  return 0;
}
