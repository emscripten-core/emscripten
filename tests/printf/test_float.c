/*
 * Copyright 2016 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */

#include <stdio.h>

int main() {
  printf("%f %f %f %f %f\n", 0.0, 0.5, 1.0, 1.75, 802.0);
  printf("%f %f %f %f %f\n", 0.01, 1.23, 14.783, 123.4567, 12345678910.0);
  printf("%f %f %f\n", 0.99, 0.8080808, 0.01020304);
  printf("%.2f %+.0e %E \n", 3.1416, 3.1416, 3.1416);
  return 0;
}
