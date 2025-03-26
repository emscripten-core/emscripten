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
  const char* source =
      "strndup - duplicate a specific number of bytes from a string";

  char* strdup_val = strndup(source, 0);
  printf("1:%s\n", strdup_val);
  free(strdup_val);

  strdup_val = strndup(source, 7);
  printf("2:%s\n", strdup_val);
  free(strdup_val);

  strdup_val = strndup(source, 1000);
  printf("3:%s\n", strdup_val);
  free(strdup_val);

  strdup_val = strndup(source, 60);
  printf("4:%s\n", strdup_val);
  free(strdup_val);

  strdup_val = strndup(source, 19);
  printf("5:%s\n", strdup_val);
  free(strdup_val);

  strdup_val = strndup(source, -1);
  printf("6:%s\n", strdup_val);
  free(strdup_val);

  return 0;
}
