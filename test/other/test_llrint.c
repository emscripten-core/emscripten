/*
 * Copyright 2016 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */

#include <stdio.h>
#include <math.h>
int main() {
  printf("%lld\n%lld\n%lld\n%lld\n", llrint(0.1), llrint(0.6), llrint(1.25),
         llrint(1099511627776.667));
  return 0;
}
