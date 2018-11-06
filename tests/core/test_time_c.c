/*
 * Copyright 2016 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */

#include <time.h>
#include <stdio.h>

int main() {
  time_t t = time(0);
  printf("time: %s\n", ctime(&t));
}
