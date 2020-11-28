/*
 * Copyright 2016 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */

//---------------
//- http://pubs.opengroup.org/onlinepubs/9699919799/functions/strndup.html
//---------------

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char** argv) {
  char* test = "12345";

  char* target = (char*)malloc(150);
  strncpy(target, test, 150);
  printf("1: %s\n", target);
  free(target);

  return 0;
}
