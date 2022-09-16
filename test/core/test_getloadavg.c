/*
 * Copyright 2016 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */

#include <stdio.h>
#include <stdlib.h>

int main() {
  double load[5] = {42.13, 42.13, 42.13, 42.13, 42.13};
  printf("ret: %d\n", getloadavg(load, 5));
  printf("load[0]: %.3lf\n", load[0]);
  printf("load[1]: %.3lf\n", load[1]);
  printf("load[2]: %.3lf\n", load[2]);
  printf("load[3]: %.3lf\n", load[3]);
  printf("load[4]: %.3lf\n", load[4]);
  return 0;
}
