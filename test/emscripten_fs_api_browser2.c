// Copyright 2015 The Emscripten Authors.  All rights reserved.
// Emscripten is available under two separate licenses, the MIT license and the
// University of Illinois/NCSA Open Source License.  Both these licenses can be
// found in the LICENSE file.

#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <emscripten.h>
#include <assert.h>
#include <string.h>
#include <SDL/SDL.h>
#include "SDL/SDL_image.h"

int get_count = 0;

void onLoaded(const char* file) {
  printf("onLoaded %s\n", file);

  assert(strcmp(file, "/tmp/test.html") == 0);

  FILE * f = fopen(file, "r");
  assert(f);
  printf("exists: %s\n", file);
  int c = fgetc(f);
  assert(c != EOF && "file empty!");
  fclose(f);

  if (++get_count == 2) {
    exit(0);
  }
}

void onError(const char* file) {
  printf("error...\n");
  assert(false);
}

int main() {
  emscripten_async_wget(
    "http://localhost:8888/test.html",
    "/tmp/test.html",
    onLoaded,
    onError);

  // get another file to the same place
  emscripten_async_wget(
    "http://localhost:8888/test.js",
    "/tmp/test.html",
    onLoaded,
    onError);

  return 99;
}
