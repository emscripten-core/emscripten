/*
 * Copyright 2014 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */

#include <stdlib.h>
#include <stdio.h>

extern char **environ;

int main(int argc, char *argv[]) {
  if (!environ) {
    puts("environ is NULL - not enough memory?");
    return 1;
  }
  for (int i = 0; environ[i] != NULL; i ++ ) {
    printf("%s\n", environ[i]);
  }
}

