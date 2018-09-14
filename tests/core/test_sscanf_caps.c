/*
 * Copyright 2016 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */

#include "stdio.h"

int main() {
  unsigned int a;
  float e, f, g;
  sscanf("a 1.1 1.1 1.1", "%X %E %F %G", &a, &e, &f, &g);
  printf("%d %.1F %.1F %.1F\n", a, e, f, g);
}
