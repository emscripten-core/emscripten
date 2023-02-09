/*
 * Copyright 2014 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <emscripten.h>

#define BUFSIZE 1024

int main() {
  const char * file = "/test.txt";
  printf("calling wget\n");
  emscripten_wget(file , file);
  printf("back from wget\n");

  printf("calling wget again to overwrite previous file\n");
  emscripten_wget(file , file);
  printf("back from wget\n");

  FILE * f = fopen(file, "r");
  assert(f);

  char buf[BUFSIZE];
  fgets(buf, BUFSIZE, f);
  buf[BUFSIZE-1] = 0;
  for(int i = 0; i < BUFSIZE; ++i) {
    buf[i] = tolower(buf[i]);
  }
  assert(strstr(buf, "emscripten"));
  fclose(f);

  printf("exiting main\n");
  // Implicit return from main with ASYNCIFY + EXIT_RUNTIME
  // currently doesn't work so we need to explicitly exit.
  // https://github.com/emscripten-core/emscripten/issues/14417
  exit(0);
}
