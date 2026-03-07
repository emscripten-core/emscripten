/*
 * Copyright 2026 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */

#include <assert.h>
#include <stdio.h>

#include <emscripten.h>

void loadScript() {
  printf("load2");
  FILE* file = fopen("file1.txt", "r");

  if (!file) {
    assert(false);
  }

  while (!feof(file)) {
    char c = fgetc(file);
    if (c != EOF) {
      putchar(c);
    }
  }
  fclose(file);
  exit(0);
}

void scriptLoadFail() {
  printf("failed to load data_files.js\n");
  assert(false);
}

int main() {
  emscripten_async_load_script("data_files.js", loadScript, scriptLoadFail);
  return 99;
}
