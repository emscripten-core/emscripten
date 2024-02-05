/*
 * Copyright 2016 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */

/* This example borrowed from MSDN documentation */
#include <stdlib.h>
#include <stdio.h>

int main() {
  int decimal, sign;
  char *buffer;
  double source = 3.1415926535;

  buffer = fcvt(source, 7, &decimal, &sign);
  printf("source: %2.10f   buffer: '%s'   decimal: %d   sign: %d\n", source,
         buffer, decimal, sign);
}
