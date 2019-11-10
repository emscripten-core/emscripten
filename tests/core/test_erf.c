/*
 * Copyright 2016 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */

#include <math.h>
#include <stdio.h>
int main() {
  printf("%1.6f, %1.6f, %1.6f, %1.6f, %1.6f, %1.6f\n", erf(1.0), erf(3.0),
         erf(-1.0), erfc(1.0), erfc(3.0), erfc(-1.5));
  return 0;
}
