// Copyright 2013 The Emscripten Authors.  All rights reserved.
// Emscripten is available under two separate licenses, the MIT license and the
// University of Illinois/NCSA Open Source License.  Both these licenses can be
// found in the LICENSE file.

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include <emscripten.h>

int value = 0;

EMSCRIPTEN_KEEPALIVE void set(int x) {
  printf("set! %d\n", x);
  value = x;
}

void load2() {
  printf("load2\n");

  char buffer[10];
  memset(buffer, 0, 10);
  FILE *f = fopen("file1.txt", "r");
  assert(f);
  fread(buffer, 1, 5, f);
  fclose(f);
  assert(strcmp(buffer, "first") == 0);

  memset(buffer, 0, 10);
  f = fopen("file2.txt", "r");
  assert(f);
  fread(buffer, 1, 6, f);
  fclose(f);
  assert(strcmp(buffer, "second") == 0);

  exit(0);
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
  return 99;
}
